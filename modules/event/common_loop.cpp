/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "common_loop.h"

#include <thread>
#include <unistd.h>
#include <signal.h>
#include <cinttypes>

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
    int event_fd = CreateEventFd();

    FdEvent *sp_read_event = newFdEvent("CommonLoop::sp_run_read_event_");
    if (!sp_read_event->initialize(event_fd, FdEvent::kReadEvent, Event::Mode::kPersist)) {
        close(event_fd);
        delete sp_read_event;
        return;
    }

    using std::placeholders::_1;
    sp_read_event->setCallback(std::bind(&CommonLoop::handleRunInLoopFunc, this));
    sp_read_event->enable();

    std::lock_guard<std::recursive_mutex> g(lock_);
    loop_thread_id_ = std::this_thread::get_id();
    run_event_fd_ = event_fd;
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
        CHECK_CLOSE_RESET_FD(run_event_fd_);
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

    if (cost > water_line_.loop_cost)
        LogNotice("loop_cost: %" PRIu64 " us", cost.count()/1000);
}

void CommonLoop::beginEventProcess()
{
    event_cb_stat_start_ = steady_clock::now();
}

void CommonLoop::endEventProcess(Event *event)
{
    auto cost = steady_clock::now() - event_cb_stat_start_;
    if (cost > water_line_.event_cb_cost)
        LogNotice("event_cb_cost: %" PRIu64 " us, what: '%s'",
                  cost.count()/1000, event->what().c_str());
}

Stat CommonLoop::getStat() const
{
    Stat stat;
    using namespace std::chrono;
    stat.stat_time_us = duration_cast<microseconds>(steady_clock::now() - whole_stat_start_).count();

    stat.loop_count = loop_count_;
    stat.loop_acc_cost_us = duration_cast<microseconds>(loop_acc_cost_).count();
    stat.loop_peak_cost_us = duration_cast<microseconds>(loop_peak_cost_).count();

    stat.run_in_loop_peak_num = run_in_loop_peak_num_;
    stat.run_next_peak_num = run_next_peak_num_;

    return stat;
}

void CommonLoop::resetStat()
{
    whole_stat_start_ = loop_stat_start_ = steady_clock::now();

    loop_count_ = 0;
    loop_acc_cost_ = nanoseconds::zero();
    loop_peak_cost_ = nanoseconds::zero();

    run_in_loop_peak_num_ = 0;
    run_next_peak_num_ = 0;
}

void CommonLoop::cleanup()
{
    cleanupDeferredTasks();
}

}
}
