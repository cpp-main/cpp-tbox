#include "signal_event.h"

#include <cassert>

#include "loop.h"
#include <tbox/base/log.h>

namespace tbox {
namespace event {

LibevSignalEvent::LibevSignalEvent(LibevLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    is_stop_after_trigger_(false),
    cb_level_(0)
{
    memset(&signal_ev_, 0, sizeof(signal_ev_));
}

LibevSignalEvent::~LibevSignalEvent()
{
    assert(cb_level_ == 0);
    disable();
}

bool LibevSignalEvent::initialize(int signum, Mode mode)
{
    disable();

    signal_ev_.active = signal_ev_.pending = 0;
    signal_ev_.priority = 0;
    signal_ev_.cb = LibevSignalEvent::OnEventCallback;
    signal_ev_.data = this;

    signal_ev_.signum = signum;

    if (mode == Mode::kOneshot) //! 如果是单次有效的，需要设置标记，使之在触发后停止事件
        is_stop_after_trigger_ = true;

    is_inited_ = true;
    return true;
}

void LibevSignalEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibevSignalEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return signal_ev_.active;
}

bool LibevSignalEvent::enable()
{
    if (!is_inited_) {
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    ev_signal_start(wp_loop_->getEvLoopPtr(), &signal_ev_);

    return true;
}

bool LibevSignalEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    ev_signal_stop(wp_loop_->getEvLoopPtr(), &signal_ev_);

    return true;
}

void LibevSignalEvent::OnEventCallback(struct ev_loop*, ev_signal *p_w, int events)
{
    assert(p_w != NULL);

    LibevSignalEvent *pthis = static_cast<LibevSignalEvent*>(p_w->data);
    pthis->onEvent();
}

void LibevSignalEvent::onEvent()
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();

    } else {
        LogErr("you should specify event callback by setCallback()");
    }

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop_->recordTimeCost(cost_us);
#endif
}

}
}
