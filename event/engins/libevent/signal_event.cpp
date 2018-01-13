#include "signal_event.h"

#include <cassert>
#include <event2/event.h>

#include "loop.h"

#include <tbox/base/log.h>

namespace tbox {
namespace event {

LibeventSignalEvent::LibeventSignalEvent(LibeventLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    cb_level_(0)
{
    event_assign(&event_, NULL, -1, 0, NULL, NULL);
}

LibeventSignalEvent::~LibeventSignalEvent()
{
    assert(cb_level_ == 0);
    disable();
}

bool LibeventSignalEvent::initialize(int signum, Mode mode)
{
    disable();

    short libevent_events = 0;
    if (mode == Mode::kPersist)
        libevent_events |= EV_PERSIST;

    int ret = event_assign(&event_, wp_loop_->getEventBasePtr(), signum,
                           libevent_events | EV_SIGNAL,
                           LibeventSignalEvent::OnEventCallback,
                           this);
    if (ret == 0) {
        is_inited_ = true;
        return true;
    }

    LogErr("event_assign() fail");
    return false;
}

void LibeventSignalEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibeventSignalEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return event_pending(&event_, EV_SIGNAL, NULL) != 0;
}

bool LibeventSignalEvent::enable()
{
    if (!is_inited_) {
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    int ret = event_add(&event_, NULL);
    if (ret != 0) {
        LogErr("event_add() fail");
        return false;
    }

    return true;
}

bool LibeventSignalEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    int ret = event_del(&event_);
    if (ret != 0) {
        LogErr("event_del() fail");
        return false;
    }

    return true;
}

void LibeventSignalEvent::OnEventCallback(int, short, void *args)
{
    LibeventSignalEvent *pthis = static_cast<LibeventSignalEvent*>(args);
    pthis->onEvent();
}

void LibeventSignalEvent::onEvent()
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;
    } else {
        LogWarn("you should specify event callback by setCallback()");
    }

    wp_loop_->handleNextFunc();

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop_->recordTimeCost(cost_us);
#endif
}

}
}
