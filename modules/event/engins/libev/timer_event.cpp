#include "timer_event.h"

#include <cassert>

#include "loop.h"
#include <tbox/base/log.h>

namespace tbox {
namespace event {

LibevTimerEvent::LibevTimerEvent(LibevLoop *wp_loop) :
    wp_loop_(wp_loop)
{
    memset(&timer_ev_, 0, sizeof(timer_ev_));
}

LibevTimerEvent::~LibevTimerEvent()
{
    assert(cb_level_ == 0);

    disable();
}

bool LibevTimerEvent::initialize(const std::chrono::milliseconds &interval, Mode mode)
{
    disable();

    timer_ev_.active = timer_ev_.pending = 0;
    timer_ev_.priority = 0;
    timer_ev_.cb = LibevTimerEvent::OnEventCallback;
    timer_ev_.data = this;

    interval_ = interval;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

void LibevTimerEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibevTimerEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return timer_ev_.active;
}

bool LibevTimerEvent::enable()
{
    if (!is_inited_) {
        //! 没有初始化，是不能直接enable的
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    double d_interval = interval_.count() * 0.001;
    if (mode_ == Mode::kOneshot) {
        timer_ev_.repeat = 0;
        timer_ev_.at = d_interval;
    } else {
        timer_ev_.repeat = timer_ev_.at = d_interval;
    }

    ev_timer_start(wp_loop_->getEvLoopPtr(), &timer_ev_);

    return true;
}

bool LibevTimerEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    ev_timer_stop(wp_loop_->getEvLoopPtr(), &timer_ev_);

    return true;
}

Loop* LibevTimerEvent::getLoop() const
{
    return wp_loop_;
}

void LibevTimerEvent::OnEventCallback(struct ev_loop*, ev_timer *p_w, int events)
{
    assert(p_w != NULL);

    LibevTimerEvent *pthis = static_cast<LibevTimerEvent*>(p_w->data);
    pthis->onEvent();
}

void LibevTimerEvent::onEvent()
{
    wp_loop_->beginEventProcess();

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;

    } else {
        LogWarn("you should specify event callback by setCallback()");
    }

    wp_loop_->endEventProcess();
}

}
}
