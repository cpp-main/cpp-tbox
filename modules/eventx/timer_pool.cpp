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
#include "timer_pool.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace eventx {

class TimerPool::Impl {
  public:
    Impl(event::Loop *wp_loop) : wp_loop_(wp_loop) { }
    ~Impl();

  public:
    TimerToken doEvery(const Milliseconds &m_sec, Callback &&cb);
    TimerToken doAfter(const Milliseconds &m_sec, Callback &&cb);
    TimerToken doAt(const TimePoint &time_point, Callback &&cb);

    bool cancel(const TimerToken &token);
    void cleanup();

  private:
    event::Loop *wp_loop_;
    cabinet::Cabinet<event::TimerEvent> timers_;
};

TimerPool::Impl::~Impl()
{
    cleanup();
}

TimerPool::TimerToken TimerPool::Impl::doEvery(const Milliseconds &m_sec, Callback &&cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return TimerToken();
    }

    auto timer = wp_loop_->newTimerEvent("TimerPool::doEvery");
    auto token = timers_.alloc(timer);
    timer->initialize(m_sec, event::Event::Mode::kPersist);
    timer->setCallback(std::move(cb));
    timer->enable();
    return token;
}

TimerPool::TimerToken TimerPool::Impl::doAfter(const Milliseconds &m_sec, Callback &&cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return TimerToken();
    }

    auto timer = wp_loop_->newTimerEvent("TimerPool::doAfter");
    auto token = timers_.alloc(timer);
    timer->initialize(m_sec, event::Event::Mode::kOneshot);

#if __cplusplus >= 201402L
    timer->setCallback([this, cb = std::move(cb), token] {
        cb();
        auto timer = timers_.free(token);
        wp_loop_->runNext([timer] { delete timer; });
    });
#elif __cplusplus >= 201103L
    timer->setCallback([this, cb, token] {
        cb();
        auto timer = timers_.free(token);
        wp_loop_->runNext([timer] { delete timer; });
    });
#endif

    timer->enable();
    return token;
}

TimerPool::TimerToken TimerPool::Impl::doAt(const TimePoint &time_point, Callback &&cb)
{
    using namespace std::chrono;
    auto d = duration_cast<Milliseconds>(time_point - system_clock::now());
    return doAfter(d, std::move(cb));
}

bool TimerPool::Impl::cancel(const TimerToken &token)
{
    auto timer = timers_.free(token);
    if (timer != nullptr) {
        timer->disable();
        if (wp_loop_->isRunning())
            wp_loop_->run([timer] { delete timer; }, "TimerPool::cancel");
        else
            delete timer;
        return true;
    }

    return false;
}

void TimerPool::Impl::cleanup()
{
    timers_.foreach(
        [this](event::TimerEvent *timer) {
            timer->disable();
            if (wp_loop_->isRunning())
                wp_loop_->run([timer]{ delete timer; }, "TimerPool::cleanup");
            else
                delete timer;
        }
    );
    timers_.clear();
}

/////////////////////////////////////////////////////////////////////////////
// wrapper
/////////////////////////////////////////////////////////////////////////////

TimerPool::TimerPool(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    TBOX_ASSERT(impl_ != nullptr);
}

TimerPool::~TimerPool()
{
    delete impl_;
}

TimerPool::TimerToken TimerPool::doEvery(const Milliseconds &m_sec, Callback &&cb)
{
    return impl_->doEvery(m_sec, std::move(cb));
}

TimerPool::TimerToken TimerPool::doAfter(const Milliseconds &m_sec, Callback &&cb)
{
    return impl_->doAfter(m_sec, std::move(cb));
}

TimerPool::TimerToken TimerPool::doAt(const TimePoint &time_point, Callback &&cb)
{
    return impl_->doAt(time_point, std::move(cb));
}

bool TimerPool::cancel(const TimerToken &token)
{
    return impl_->cancel(token);
}

void TimerPool::cleanup()
{
    impl_->cleanup();
}

}
}
