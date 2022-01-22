#include "timers.h"

#include <cassert>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox::eventx {

class Timers::Impl {
  public:
    Impl(event::Loop *wp_loop) : wp_loop_(wp_loop) { }
    ~Impl();

  public:
    Token doEvery(const Milliseconds &m_sec, const Callback &cb);
    Token doAfter(const Milliseconds &m_sec, const Callback &cb);
    Token doAt(const TimePoint &time_point, const Callback &cb);

    bool cancel(const Token &token);
    void cleanup();

  private:
    event::Loop *wp_loop_;
    cabinet::Cabinet<event::TimerEvent> timers_;
};

Timers::Impl::~Impl()
{
    cleanup();
}

Timers::Token Timers::Impl::doEvery(const Milliseconds &m_sec, const Callback &cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return Token();
    }

    auto new_timer = wp_loop_->newTimerEvent();
    auto new_token = timers_.insert(new_timer);
    new_timer->initialize(m_sec, event::Event::Mode::kPersist);
    new_timer->setCallback(std::bind(cb, new_token));
    new_timer->enable();
    return new_token;
}

Timers::Token Timers::Impl::doAfter(const Milliseconds &m_sec, const Callback &cb)
{
    if (!cb) {
        LogWarn("cb == nullptr");
        return Token();
    }

    auto new_timer = wp_loop_->newTimerEvent();
    auto new_token = timers_.insert(new_timer);
    new_timer->initialize(m_sec, event::Event::Mode::kOneshot);
    new_timer->setCallback(
        [new_token, cb, this] {
            cb(new_token);
            cancel(new_token);
        }
    );
    new_timer->enable();
    return new_token;
}

Timers::Token Timers::Impl::doAt(const TimePoint &time_point, const Callback &cb)
{
    using namespace std::chrono;
    auto d = duration_cast<Milliseconds>(time_point - system_clock::now());
    return doAfter(d, cb);
}

bool Timers::Impl::cancel(const Token &token)
{
    auto timer = timers_.remove(token);
    if (timer != nullptr) {
        timer->disable();
        if (wp_loop_->isRunning())
            wp_loop_->runNext([timer] { delete timer; });
        else
            delete timer;
        return true;
    }

    return false;
}

void Timers::Impl::cleanup()
{
    timers_.foreach(
        [](event::TimerEvent *timer) {
            timer->disable();
            delete timer;
        }
    );
    timers_.clear();
}

/////////////////////////////////////////////////////////////////////////////
// wrapper
/////////////////////////////////////////////////////////////////////////////

Timers::Timers(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    assert(impl_ != nullptr);
}

Timers::~Timers()
{
    delete impl_;
}

Timers::Token Timers::doEvery(const Milliseconds &m_sec, const Callback &cb)
{
    return impl_->doEvery(m_sec, cb);
}

Timers::Token Timers::doAfter(const Milliseconds &m_sec, const Callback &cb)
{
    return impl_->doAfter(m_sec, cb);
}

Timers::Token Timers::doAt(const TimePoint &time_point, const Callback &cb)
{
    return impl_->doAt(time_point, cb);
}

bool Timers::cancel(const Token &token)
{
    return impl_->cancel(token);
}

void Timers::cleanup()
{
    impl_->cleanup();
}

}
