#include "common_loop.h"

#include <thread>
#include <unistd.h>
#include <signal.h>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>

#include "fd_event.h"
#include "stat.h"
#include "misc.h"

namespace tbox {
namespace event {

using namespace std::chrono;

CommonLoop::~CommonLoop()
{
    TBOX_ASSERT(cb_level_ == 0);
}

bool CommonLoop::isInLoopThread()
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    return isInLoopThreadLockless();
}

bool CommonLoop::isRunning() const
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    return isRunningLockless();
}

bool CommonLoop::isInLoopThreadLockless() const
{
    return std::this_thread::get_id() == loop_thread_id_;
}

bool CommonLoop::isRunningLockless() const
{
    return sp_run_read_event_ != nullptr;
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
    sp_read_event->setCallback(std::bind(&CommonLoop::handleRunInLoopRequest, this, _1));
    sp_read_event->enable();

    std::lock_guard<std::recursive_mutex> g(lock_);
    loop_thread_id_ = std::this_thread::get_id();
    run_read_fd_ = read_fd;
    run_write_fd_ = write_fd;
    sp_run_read_event_ = sp_read_event;

    if (!run_in_loop_func_queue_.empty())
        commitRunRequest();

#ifdef  ENABLE_STAT
    resetStat();
#endif
}

void CommonLoop::runThisAfterLoop()
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    cleanupDeferredTasks();

    loop_thread_id_ = std::thread::id();    //! 清空 loop_thread_id_
    if (sp_run_read_event_ != nullptr) {
        CHECK_DELETE_RESET_OBJ(sp_run_read_event_);
        CHECK_CLOSE_RESET_FD(run_write_fd_);
        CHECK_CLOSE_RESET_FD(run_read_fd_);
    }
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
