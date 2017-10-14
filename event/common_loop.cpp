#include "common_loop.h"

#include <mutex>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

#include <tbox/base/log.h>

#include "fd_item.h"
#include "stat.h"

namespace tbox {
namespace event {

CommonLoop::CommonLoop() :
    has_unhandle_req_(false),
    read_fd_(-1), write_fd_(-1),
    sp_read_item_(NULL),
    cb_level_(0)
{ }

CommonLoop::~CommonLoop()
{
    assert(cb_level_ == 0);
}

bool CommonLoop::isInLoopThread()
{
    std::lock_guard<std::mutex> g(lock_);
    return std::this_thread::get_id() == loop_thread_id_;
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

    FdItem *sp_read_item = newFdItem();
    if (!sp_read_item->initialize(read_fd, FdItem::kReadEvent, Item::Mode::kPersist)) {
        close(write_fd);
        close(read_fd);
        delete sp_read_item;
        return;
    }

    using std::placeholders::_1;
    sp_read_item->setCallback(std::bind(&CommonLoop::onGotRunInLoopFunc, this, _1));
    sp_read_item->enable();

    std::lock_guard<std::mutex> g(lock_);
    loop_thread_id_ = std::this_thread::get_id();
    read_fd_ = read_fd;
    write_fd_ = write_fd;
    sp_read_item_ = sp_read_item;

    if (!func_list_.empty())
        commitRequest();

#ifdef  ENABLE_STAT
    resetStat();
#endif
}

void CommonLoop::runThisAfterLoop()
{
    std::lock_guard<std::mutex> g(lock_);
    loop_thread_id_ = std::thread::id();

    if (sp_read_item_ != NULL) {
        delete sp_read_item_;
        close(write_fd_);
        close(read_fd_);

        sp_read_item_ = NULL;
        write_fd_ = -1;
        read_fd_ = -1;
    }
}

void CommonLoop::runInLoop(const RunInLoopFunc &func)
{
    std::lock_guard<std::mutex> g(lock_);
    func_list_.push_back(func);

    if (sp_read_item_ == NULL)
        return;

    commitRequest();
}

void CommonLoop::onGotRunInLoopFunc(short)
{
    std::list<RunInLoopFunc> tmp;
    {
        std::lock_guard<std::mutex> g(lock_);
        func_list_.swap(tmp);
        finishRequest();
    }

    while (!tmp.empty()) {
        RunInLoopFunc func = tmp.front();
        ++cb_level_;
        if (func)
            func();
        --cb_level_;
        tmp.pop_front();
    }
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

Stat CommonLoop::getStat() const
{
    Stat stat;
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    stat.stat_time_us = duration_cast<microseconds>(steady_clock::now() - stat_start_).count();
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
    stat_start_ = steady_clock::now();
#endif
}

#ifdef  ENABLE_STAT
void CommonLoop::recordTimeCost(uint64_t cost_us)
{
    ++event_count_;
    time_cost_us_ += cost_us;
    if (max_cost_us_ < cost_us)
        max_cost_us_ = cost_us;
}
#endif  //ENABLE_STAT

}
}

#ifdef ENABLE_TEST
#include <gtest/gtest.h>

TEST(CommonLoop, _)
{ }

#endif //ENABLE_TEST

