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
#include "timer_event.h"

namespace tbox {
namespace event {

using namespace std::chrono;

CommonLoop::~CommonLoop()
{
    TBOX_ASSERT(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
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

    resetStat();
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

void CommonLoop::beginLoopProcess()
{
    loop_stat_start_ = steady_clock::now();
}

void CommonLoop::endLoopProcess()
{
    auto cost = steady_clock::now() - loop_stat_start_;
    ++loop_count_;
    loop_acc_cost_ += cost;
    if (loop_peak_cost_ < cost)
        loop_peak_cost_ = cost;
}

void CommonLoop::beginEventProcess()
{
     event_stat_start_ = steady_clock::now();
}

void CommonLoop::endEventProcess()
{
    auto cost = steady_clock::now() - event_stat_start_;
    ++event_count_;
    event_acc_cost_ += cost;
    if (event_peak_cost_ < cost)
        event_peak_cost_ = cost;

    auto cost_us = duration_cast<microseconds>(cost).count();
    if (cost_us > cb_time_cost_threshold_us_)
        LogWarn("cost_us: %u", cost_us);
}

Stat CommonLoop::getStat() const
{
    Stat stat;
    using namespace std::chrono;
    stat.stat_time_us = duration_cast<microseconds>(steady_clock::now() - whole_stat_start_).count();

    stat.event_count = event_count_;
    stat.event_acc_cost_us = duration_cast<microseconds>(event_acc_cost_).count();
    stat.event_peak_cost_us = duration_cast<microseconds>(event_peak_cost_).count();

    stat.loop_count = loop_count_;
    stat.loop_acc_cost_us = duration_cast<microseconds>(loop_acc_cost_).count();
    stat.loop_peak_cost_us = duration_cast<microseconds>(loop_peak_cost_).count();

    stat.run_in_loop_peak_num = run_in_loop_peak_num_;
    stat.run_next_peak_num = run_next_peak_num_;

    return stat;
}

void CommonLoop::resetStat()
{
    event_stat_start_ = whole_stat_start_ = loop_stat_start_ = steady_clock::now();

    event_count_ = 0;
    event_acc_cost_ = nanoseconds::zero();
    event_peak_cost_ = nanoseconds::zero();

    loop_count_ = 0;
    loop_acc_cost_ = nanoseconds::zero();
    loop_peak_cost_ = nanoseconds::zero();

    run_in_loop_peak_num_ = 0;
    run_next_peak_num_ = 0;
}

void CommonLoop::setCBTimeCostThreshold(uint32_t threshold_us)
{
    cb_time_cost_threshold_us_ = threshold_us;
}

}
}
