#include "timer_pool.h"

#include <cassert>

#include <tbox/base/log.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox::eventx {

class TimerPool::Impl {
  public:
    Impl(event::Loop *wp_loop) : wp_loop_(wp_loop) { }
    ~Impl();

  public:
    TimerToken doEvery(const Milliseconds &m_sec, const Callback &cb);
    TimerToken doAfter(const Milliseconds &m_sec, const Callback &cb);
    TimerToken doAt(const TimePoint &time_point, const Callback &cb);

    bool cancel(const TimerToken &token);
    void cleanup();

  private:
    event::Loop *wp_loop_;
    cabinet::Cabinet<event::TimerEvent> timers_;
    int cb_level_ = 0;
};

TimerPool::Impl::~Impl()
{
    assert(cb_level_ == 0);
    cleanup();
}

TimerPool::TimerToken TimerPool::Impl::doEvery(const Milliseconds &m_sec, const Callback &cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return TimerToken();
    }

    auto new_timer = wp_loop_->newTimerEvent();
    auto new_token = timers_.insert(new_timer);
    new_timer->initialize(m_sec, event::Event::Mode::kPersist);
    new_timer->setCallback(
        [new_token, cb, this] {
            ++cb_level_;
            cb(new_token);
            --cb_level_;
        }
    );
    new_timer->enable();
    return new_token;
}

TimerPool::TimerToken TimerPool::Impl::doAfter(const Milliseconds &m_sec, const Callback &cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return TimerToken();
    }

    auto new_timer = wp_loop_->newTimerEvent();
    auto new_token = timers_.insert(new_timer);
    new_timer->initialize(m_sec, event::Event::Mode::kOneshot);
    new_timer->setCallback(
        [new_token, cb, this] {
            ++cb_level_;
            cb(new_token);
            --cb_level_;
            cancel(new_token);
        }
    );
    new_timer->enable();
    return new_token;
}

TimerPool::TimerToken TimerPool::Impl::doAt(const TimePoint &time_point, const Callback &cb)
{
    using namespace std::chrono;
    auto d = duration_cast<Milliseconds>(time_point - steady_clock::now());
    return doAfter(d, cb);
}

bool TimerPool::Impl::cancel(const TimerToken &token)
{
    auto timer = timers_.remove(token);
    if (timer != nullptr) {
        timer->disable();
        if (wp_loop_->isRunning())
            wp_loop_->run([timer] { delete timer; });
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
                wp_loop_->run([timer]{ delete timer; });
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
    assert(impl_ != nullptr);
}

TimerPool::~TimerPool()
{
    delete impl_;
}

TimerPool::TimerToken TimerPool::doEvery(const Milliseconds &m_sec, const Callback &cb)
{
    return impl_->doEvery(m_sec, cb);
}

TimerPool::TimerToken TimerPool::doAfter(const Milliseconds &m_sec, const Callback &cb)
{
    return impl_->doAfter(m_sec, cb);
}

TimerPool::TimerToken TimerPool::doAt(const TimePoint &time_point, const Callback &cb)
{
    return impl_->doAt(time_point, cb);
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
