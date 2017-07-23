#include "signal_item.h"

#include <cassert>
#include <event2/event.h>

#include "loop.h"

#include <tbox/log.h>

namespace tbox {
namespace event {

LibeventSignalItem::LibeventSignalItem(LibeventLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    cb_level_(0)
{
    event_assign(&event_, NULL, -1, 0, NULL, NULL);
}

LibeventSignalItem::~LibeventSignalItem()
{
    assert(cb_level_ == 0);
    disable();
}

bool LibeventSignalItem::initialize(int signum, Mode mode)
{
    disable();

    short libevent_events = 0;
    if (mode == Mode::kPersist)
        libevent_events |= EV_PERSIST;

    int ret = event_assign(&event_, wp_loop_->getEventBasePtr(), signum,
                           libevent_events | EV_SIGNAL,
                           LibeventSignalItem::OnEventCallback,
                           this);
    if (ret == 0) {
        is_inited_ = true;
        return true;
    }

    LogErr("event_assign() fail");
    return false;
}

void LibeventSignalItem::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibeventSignalItem::isEnabled() const
{
    if (!is_inited_)
        return false;

    return event_pending(&event_, EV_SIGNAL, NULL) != 0;
}

bool LibeventSignalItem::enable()
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

bool LibeventSignalItem::disable()
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

void LibeventSignalItem::OnEventCallback(int, short, void *args)
{
    LibeventSignalItem *pthis = static_cast<LibeventSignalItem*>(args);
    pthis->onEvent();
}

void LibeventSignalItem::onEvent()
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
