#include "timer_item.h"

#include <cassert>
#include <event2/event.h>

#include "loop.h"

#include <tbox/log.h>

namespace tbox {
namespace event {

LibeventTimerItem::LibeventTimerItem(LibeventLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    cb_level_(0)
{
    event_assign(&event_, NULL, -1, 0, NULL, NULL);
}

LibeventTimerItem::~LibeventTimerItem()
{
    assert(cb_level_ == 0);
    disable();
}

bool LibeventTimerItem::initialize(const Timespan &interval, Mode mode)
{
    disable();

    interval_ = interval;

    short libevent_events = 0;
    if (mode == Mode::kPersist)
        libevent_events |= EV_PERSIST;

    int ret = event_assign(&event_, wp_loop_->getEventBasePtr(), -1, libevent_events, LibeventTimerItem::OnEventCallback, this);
    if (ret == 0) {
        is_inited_ = true;
        return true;
    }

    LogErr("event_assign() fail");
    return false;
}

void LibeventTimerItem::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibeventTimerItem::isEnabled() const
{
    if (!is_inited_)
        return false;

    return event_pending(&event_, EV_TIMEOUT, NULL) != 0;
}

bool LibeventTimerItem::enable()
{
    if (!is_inited_) {
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    struct timeval tv = interval_;
    int ret = event_add(&event_, &tv);
    if (ret != 0) {
        LogErr("event_add() fail");
        return false;
    }

    return true;
}

bool LibeventTimerItem::disable()
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

void LibeventTimerItem::OnEventCallback(int, short, void *args)
{
    LibeventTimerItem *pthis = static_cast<LibeventTimerItem*>(args);
    pthis->onEvent();
}

void LibeventTimerItem::onEvent()
{
    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;
    } else {
        LogWarn("you should specify event callback by setCallback()");
    }
}

}
}
