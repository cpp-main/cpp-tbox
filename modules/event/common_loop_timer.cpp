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

#include <algorithm>
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>

#include "timer_event_impl.h"

namespace tbox {
namespace event {

namespace {
uint64_t GetCurrentSteadyClockMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds> \
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}
}

int64_t CommonLoop::getWaitTime() const
{
    if (hasNextFunc())
        return 0;

    /// Get the top of minimum heap
    int64_t wait_time = -1;
    if (!timer_min_heap_.empty()) {
        wait_time = timer_min_heap_.front()->expired - GetCurrentSteadyClockMilliseconds();
        if (wait_time < 0) //! If expired is little than now, then we consider this timer invalid and trigger it immediately.
            wait_time = 0;
    }

    return wait_time;
}

void CommonLoop::handleExpiredTimers()
{
    RECORD_SCOPE();

    auto now = GetCurrentSteadyClockMilliseconds();

    while (!timer_min_heap_.empty()) {
        auto t = timer_min_heap_.front();
        //TBOX_ASSERT(t != nullptr);

        if (now < t->expired)
            break;

        int delay_ms = now - t->expired;
        if (delay_ms > (water_line_.timer_delay.count() / 1000000))
            LogNotice("timer delay over waterline: %d ms", delay_ms);

        auto tobe_run = t->cb;

        // swap first element and last element
        std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
        if (UNLIKELY(t->repeat == 1)) {
            // remove the last element
            timer_min_heap_.pop_back();
            timer_cabinet_.free(t->token);
            timer_object_pool_.free(t);
        } else {
            t->expired += t->interval;
            // push the last element to heap again
            std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
            if (LIKELY(t->repeat != 0))
                --t->repeat;
        }

        //! Q: 为什么不在L68执行？
        //! A: 因为要尽可能地将回调放到最后执行。否则不满足测试 TEST(TimerEvent, DisableSelfInCallback)
        if (LIKELY(tobe_run)) {
            RECORD_SCOPE();
            ++cb_level_;
            tobe_run();
            --cb_level_;
        }
    }
}

void CommonLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (sp_exit_timer_ != nullptr) {
        sp_exit_timer_->disable();
        CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
    }

    if (wait_time.count() == 0) {
        stopLoop();
    } else {
        sp_exit_timer_ = newTimerEvent(__func__);
        sp_exit_timer_->initialize(wait_time, Event::Mode::kOneshot);
        sp_exit_timer_->setCallback([this] { stopLoop(); });
        sp_exit_timer_->enable();
    }
}

cabinet::Token CommonLoop::addTimer(uint64_t interval, uint64_t repeat, const TimerCallback &cb)
{
    TBOX_ASSERT(cb);

    auto now = GetCurrentSteadyClockMilliseconds();

    Timer *t = timer_object_pool_.alloc();
    TBOX_ASSERT(t != nullptr);

    t->token = this->timer_cabinet_.alloc(t);

    t->expired = now + interval;
    t->interval = interval;
    t->cb = cb;
    t->repeat = repeat;

    timer_min_heap_.push_back(t);
    std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());

    return t->token;
}

void CommonLoop::deleteTimer(const cabinet::Token& token)
{
    auto timer = timer_cabinet_.free(token);
    if (timer == nullptr)
        return;

#if 0
    timer_min_heap_.erase(timer);
    std::make_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
#else
    //! If we use the above method, it is likely to disrupt order, leading to a wide range of exchanges.
    //! This method will be a little better.
    timer->expired = 0;
    std::make_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
    std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
    timer_min_heap_.pop_back();
#endif

    run([this, timer] { timer_object_pool_.free(timer); }, __func__); //! Delete later, avoid delete itself
}

TimerEvent* CommonLoop::newTimerEvent(const std::string &what)
{
    return new TimerEventImpl(this, what);
}

}
}
