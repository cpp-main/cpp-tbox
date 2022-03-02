#include "common_loop.h"

#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cassert>
#include <signal.h>

#include <tbox/base/log.h>

#include "fd_event.h"
#include "stat.h"

namespace tbox {
namespace event {

using namespace std::chrono;

std::map<int, std::set<int>> CommonLoop::_signal_read_fds_;
std::mutex CommonLoop::_signal_lock_;

CommonLoop::CommonLoop() :
    has_unhandle_req_(false),
    read_fd_(-1), write_fd_(-1),
    sp_read_event_(nullptr),
    cb_level_(0)
{ }

CommonLoop::~CommonLoop()
{
    assert(cb_level_ == 0);

    std::lock_guard<std::recursive_mutex> g(lock_);
    cleanupDeferredTasks();
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
    int fds[2] = { 0 };
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {  //!FIXME
        LogErr("pip2() fail, ret:%d", errno);
        return;
    }

    int read_fd(fds[0]);
    int write_fd(fds[1]);

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
        delete sp_read_event_;
        close(write_fd_);
        close(read_fd_);

        sp_read_event_ = nullptr;
        write_fd_ = -1;
        read_fd_ = -1;
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
        int fds[2] = { 0 };
        if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {  //!FIXME
            LogErr("pip2() fail, ret:%d", errno);
            return false;
        }
        int read_fd(fds[0]);
        int write_fd(fds[1]);

        auto read_fd_event = newFdEvent();
        read_fd_event->initialize(read_fd, FdEvent::kReadEvent, Event::Mode::kPersist);
        read_fd_event->setCallback(std::bind(&CommonLoop::onSignal, this));
        read_fd_event->enable();

        {
            std::unique_lock<std::mutex> _g(_signal_lock_);
            auto iter = _signal_read_fds_.find(signo);
            if (iter != _signal_read_fds_.end()) {
                iter->second.insert(read_fd);
            } else {
                std::set<int> tmp = { read_fd };
                _signal_read_fds_[signo] = std::move(tmp);
                signal(signo, CommonLoop::HandleSignal);
            }
        }

        signal_read_fd_  = read_fd;
        signal_write_fd_ = write_fd;
        sp_signal_read_event_ = read_fd_event;
    }

    auto iter = signal_subscribers_.find(signo);
    if (iter != signal_subscribers_.end()) {
        iter->second.insert(who);
    } else {
        std::set<SignalSubscribuer*> tmp = { who };
        signal_subscribers_[signo] = std::move(tmp);
    }

    return true;
}

bool CommonLoop::unsubscribeSignal(int signo, SignalSubscribuer *who)
{
    auto iter = signal_subscribers_.find(signo);
    if (iter == signal_subscribers_.end())  //! 如果本来就不存在，就直接返回了
        return true;

    auto &subscriber_set = iter->second;
    subscriber_set.erase(who);          //! 将订阅信息删除

    if (!subscriber_set.empty())        //! 检查本Loop中是否已经没有SignalSubscribuer订阅该信号了
        return true;    //! 如果还有，就到此为止

    //! 如果本Loop已经没有SignalSubscribuer订阅该信号了
    signal_subscribers_.erase(iter);    //! 则将该信号的订阅记录表删除

    std::unique_lock<std::mutex> _g(_signal_lock_);
    //! 并将 _signal_read_fds_ 中的记录删除
    auto fd_iter = _signal_read_fds_.find(signo);
    assert(fd_iter != _signal_read_fds_.end());

    auto &fd_set = fd_iter->second;
    fd_set.erase(signal_read_fd_);

    //! 检查是否还有其它的Loop订阅该信号
    if (!fd_set.empty())
        return true;

    //! 如果没有了，则删除 _signal_read_fds_ 中该信号的订阅记录表
    _signal_read_fds_.erase(fd_iter);

    //! 并还原信号处理函数
    signal(signo, SIG_DFL);
    return true;
}

void CommonLoop::HandleSignal(int signo)
{
    //std::unique_lock<std::mutex> _g(_signal_lock_); //!FIXME: 这是在信号中执行的，是否需要加锁？
    auto iter = _signal_read_fds_.find(signo);
    if (iter != _signal_read_fds_.end()) {
        const auto &fd_set = iter->second;
        for (int fd : fd_set)
            write(fd, &signo, sizeof(signo));
    } else {
        LogWarn("uncatch signal: %d", signo);
    }
}

void CommonLoop::onSignal()
{
    while (true) {
        int signo = 0;
        auto rsize = read(signal_read_fd_, &signo, sizeof(signo));
        if (rsize > 0) {
            auto iter = signal_subscribers_.find(signo);
            if (iter != signal_subscribers_.end()) {
                for (auto s : iter->second) {
                    s->onSignal(signo);
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
