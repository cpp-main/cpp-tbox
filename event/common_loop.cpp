#include "common_loop.h"

#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cassert>
#include <signal.h>

#include <tbox/base/log.h>
#include <tbox/base/defines.h>

#include "fd_event.h"
#include "stat.h"

namespace tbox {
namespace event {

using namespace std::chrono;

std::map<int, std::set<int>> CommonLoop::_signal_write_fds_;
std::mutex    CommonLoop::_signal_lock_;

namespace {
bool CreateFdPair(int &read_fd, int &write_fd)
{
    int fds[2] = { 0 };
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {  //!FIXME
        LogErr("pip2() fail, ret:%d", errno);
        return false;
    }

    read_fd = fds[0];
    write_fd = fds[1];
    return true;
}
}

CommonLoop::CommonLoop() :
    has_unhandle_req_(false),
    read_fd_(-1), write_fd_(-1),
    sp_read_event_(nullptr),
    cb_level_(0)
{ }

CommonLoop::~CommonLoop()
{
    assert(cb_level_ == 0);
}

bool CommonLoop::isInLoopThread()
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    return std::this_thread::get_id() == loop_thread_id_;
}

bool CommonLoop::isRunning() const
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    return sp_read_event_ != nullptr;
}

void CommonLoop::runThisBeforeLoop()
{
    int read_fd = -1, write_fd = -1;
    if (!CreateFdPair(read_fd, write_fd))
        return;

    FdEvent *sp_read_event = newFdEvent();
    if (!sp_read_event->initialize(read_fd, FdEvent::kReadEvent, Event::Mode::kPersist)) {
        close(write_fd);
        close(read_fd);
        delete sp_read_event;
        return;
    }

    using std::placeholders::_1;
    sp_read_event->setCallback(std::bind(&CommonLoop::onGotRunInLoopFunc, this, _1));
    sp_read_event->enable();

    std::lock_guard<std::recursive_mutex> g(lock_);
    loop_thread_id_ = std::this_thread::get_id();
    read_fd_ = read_fd;
    write_fd_ = write_fd;
    sp_read_event_ = sp_read_event;

    if (!run_in_loop_func_queue_.empty())
        commitRequest();

#ifdef  ENABLE_STAT
    resetStat();
#endif
}

void CommonLoop::runThisAfterLoop()
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    cleanupDeferredTasks();

    loop_thread_id_ = std::thread::id();    //! 清空 loop_thread_id_
    if (sp_read_event_ != nullptr) {
        CHECK_DELETE_RESET_OBJ(sp_read_event_);
        CHECK_CLOSE_RESET_FD(write_fd_);
        CHECK_CLOSE_RESET_FD(read_fd_);
    }
}

void CommonLoop::runInLoop(const Func &func)
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    run_in_loop_func_queue_.push_back(func);

    if (sp_read_event_ == nullptr)
        return;

    commitRequest();
}

void CommonLoop::runNext(const Func &func)
{
    if (!isInLoopThread()) {
        LogWarn("Fail, use runInLoop() instead.");
        return;
    }

    run_next_func_queue_.push_back(func);
}

void CommonLoop::run(const Func &func)
{
    if (isInLoopThread())
        run_next_func_queue_.push_back(func);
    else
        runInLoop(func);
}

void CommonLoop::beginEventProcess()
{
#ifdef  ENABLE_STAT
    if (stat_enable_)
        event_stat_start_ = steady_clock::now();
#endif
}

void CommonLoop::endEventProcess()
{
    handleNextFunc();

#ifdef  ENABLE_STAT
    if (stat_enable_) {
        uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - event_stat_start_).count();
        ++event_count_;
        time_cost_us_ += cost_us;
        if (max_cost_us_ < cost_us)
            max_cost_us_ = cost_us;
    }
#endif
}

bool CommonLoop::subscribeSignal(int signo, SignalSubscribuer *who)
{
    if (signal_read_fd_ == -1) {    //! 如果还没有创建对应的信号
        if (!CreateFdPair(signal_read_fd_, signal_write_fd_))
            return false;

        sp_signal_read_event_ = newFdEvent();
        sp_signal_read_event_->initialize(signal_read_fd_, FdEvent::kReadEvent, Event::Mode::kPersist);
        sp_signal_read_event_->setCallback(std::bind(&CommonLoop::onSignal, this));
        sp_signal_read_event_->enable();
    }

    auto &this_signal_subscribers = all_signals_subscribers_[signo];
    if (this_signal_subscribers.empty()) {
        //! 如果本Loop没有监听该信号，则要去 _signal_write_fds_ 中订阅
        std::unique_lock<std::mutex> _g(_signal_lock_);
        //!FIXME: 要禁止信号触发
        auto & signo_fds = _signal_write_fds_[signo];
        if (signo_fds.empty())
            signal(signo, CommonLoop::HandleSignal);
        signo_fds.insert(signal_write_fd_);
    }
    this_signal_subscribers.insert(who);

    return true;
}

bool CommonLoop::unsubscribeSignal(int signo, SignalSubscribuer *who)
{
    auto &this_signal_subscribers = all_signals_subscribers_[signo];
    this_signal_subscribers.erase(who);          //! 将订阅信息删除
    if (!this_signal_subscribers.empty())        //! 检查本Loop中是否已经没有SignalSubscribuer订阅该信号了
        return true;    //! 如果还有，就到此为止

    //! 如果本Loop已经没有SignalSubscribuer订阅该信号了
    all_signals_subscribers_.erase(signo);    //! 则将该信号的订阅记录表删除
    {
        std::unique_lock<std::mutex> _g(_signal_lock_);
        //!FIXME: 要禁止信号触发
        //! 并将 _signal_write_fds_ 中的记录删除
        auto &this_signal_fds = _signal_write_fds_[signo];
        this_signal_fds.erase(signal_write_fd_);
        if (this_signal_fds.empty()) {
            //! 并还原信号处理函数
            signal(signo, SIG_DFL);
            _signal_write_fds_.erase(signo);
        }
    }

    if (!all_signals_subscribers_.empty())
        return true;

    //! 已经没有任何SignalSubscribuer订阅任何信号了
    sp_signal_read_event_->disable();
    CHECK_CLOSE_RESET_FD(signal_write_fd_);
    CHECK_CLOSE_RESET_FD(signal_read_fd_);

    FdEvent *tobe_delete = nullptr;
    std::swap(tobe_delete, sp_signal_read_event_);
    run([tobe_delete] { delete tobe_delete; });

    return true;
}

void CommonLoop::HandleSignal(int signo)
{
    auto &this_signal_fds = _signal_write_fds_[signo];
    for (int fd : this_signal_fds)
        write(fd, &signo, sizeof(signo));
}

void CommonLoop::onSignal()
{
    while (signal_read_fd_ != -1) {
        int signo_array[10];    //! 一次性读10个
        auto rsize = read(signal_read_fd_, &signo_array, sizeof(signo_array));
        if (rsize > 0) {
            const auto num = rsize / sizeof(int);
            for (size_t i = 0; i < num; ++i) {
                int signo = signo_array[i];
                auto iter = all_signals_subscribers_.find(signo);
                if (iter != all_signals_subscribers_.end()) {
                    for (auto s : iter->second) {
                        s->onSignal(signo);
                    }
                }
            }
        } else {
            if (errno != EAGAIN)
                LogWarn("read error, rsize:%d, errno:%d, %s", rsize, errno, strerror(errno));
            break;
        }
    }
}

SignalEvent* CommonLoop::newSignalEvent()
{
    return new SignalEventImpl(this);
}

void CommonLoop::handleNextFunc()
{
    while (!run_next_func_queue_.empty()) {
        Func &func = run_next_func_queue_.front();
        if (func) {
            ++cb_level_;
            func();
            --cb_level_;
        }
        run_next_func_queue_.pop_front();
    }
}

void CommonLoop::onGotRunInLoopFunc(short)
{
    /**
     * NOTICE:
     * 这里使用 tmp 将 run_in_loop_func_queue_ 中的内容交换出去。然后再从 tmp 逐一取任务出来执行。
     * 其目的在于腾空 run_in_loop_func_queue_，让新 runInLoop() 的任务则会在下一轮循环中执行。
     * 从而防止无限 runInLoop() 引起的死循环，导致其它事件得不到处理。
     *
     * 这点与 runNext() 不同
     */
    std::deque<Func> tmp;
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        run_in_loop_func_queue_.swap(tmp);
        finishRequest();
    }

    while (!tmp.empty()) {
        Func &func = tmp.front();
        if (func) {
            ++cb_level_;
            func();
            --cb_level_;

            handleNextFunc();
        }
        tmp.pop_front();
    }
}

//! 清理 run_in_loop_func_queue_ 与 run_next_func_queue_ 中的任务
void CommonLoop::cleanupDeferredTasks()
{
    int remain_loop_count = 10; //! 限定次数，防止出现 runNext() 递归导致无法退出循环的问题
    while ((!run_in_loop_func_queue_.empty() || !run_next_func_queue_.empty())
            && remain_loop_count-- > 0) {
        std::deque<Func> tasks = std::move(run_next_func_queue_);
        tasks.insert(tasks.end(), run_in_loop_func_queue_.begin(), run_in_loop_func_queue_.end());
        run_in_loop_func_queue_.clear();

        while (!tasks.empty()) {
            Func &func = tasks.front();
            if (func) {
                ++cb_level_;
                func();
                --cb_level_;
            }
            tasks.pop_front();
        }
    }

    if (remain_loop_count == 0)
        LogWarn("found recursive actions, force quit");
}

void CommonLoop::commitRequest()
{
    if (!has_unhandle_req_) {
        char ch = 0;
        ssize_t wsize = write(write_fd_, &ch, 1);
        (void)wsize;

        has_unhandle_req_ = true;
    }
}

void CommonLoop::finishRequest()
{
    char ch = 0;
    ssize_t rsize = read(read_fd_, &ch, 1);
    (void)rsize;

    has_unhandle_req_ = false;
}

void CommonLoop::setStatEnable(bool enable)
{
#ifdef  ENABLE_STAT
    if (!stat_enable_ && enable)
        resetStat();

    stat_enable_ = enable;
#endif
}

bool CommonLoop::isStatEnabled() const
{
#ifdef  ENABLE_STAT
    return stat_enable_;
#else
    return false;
#endif
}

Stat CommonLoop::getStat() const
{
    Stat stat;
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    stat.stat_time_us = duration_cast<microseconds>(steady_clock::now() - whole_stat_start_).count();
    stat.time_cost_us = time_cost_us_;
    stat.max_cost_us = max_cost_us_;
    stat.event_count = event_count_;
#endif
    return stat;
}

void CommonLoop::resetStat()
{
#ifdef  ENABLE_STAT
    time_cost_us_ = max_cost_us_ = event_count_ = 0;
    event_stat_start_ = whole_stat_start_ = steady_clock::now();
#endif
}

}
}
