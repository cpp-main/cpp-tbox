#include "fd_item.h"

#include <cassert>
#include <event2/event.h>
#include <tbox/base/log.h>

#include "loop.h"

namespace tbox {
namespace event {

LibeventFdItem::LibeventFdItem(LibeventLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    events_(0),
    cb_level_(0)
{
    event_assign(&event_, NULL, -1, 0, NULL, NULL);
}

LibeventFdItem::~LibeventFdItem()
{
    assert(cb_level_ == 0);
    disable();
}

namespace {
short LibeventEventsToLocal(short libevent_events)
{
    short ret = 0;
    if (libevent_events & EV_READ)
        ret |= FdItem::kReadEvent;
    if (libevent_events & EV_WRITE)
        ret |= FdItem::kWriteEvent;

    return ret;
}

short LocalEventsToLibevent(short local_events)
{
    short ret = 0;
    if (local_events & FdItem::kWriteEvent)
        ret |= EV_WRITE;
    if (local_events & FdItem::kReadEvent)
        ret |= EV_READ;

    return ret;
}

}

bool LibeventFdItem::initialize(int fd, short events, Mode mode)
{
    disable();

    short libevent_events = LocalEventsToLibevent(events);
    if (mode == Mode::kPersist)
        libevent_events |= EV_PERSIST;

    int ret = event_assign(&event_, wp_loop_->getEventBasePtr(), fd, libevent_events, LibeventFdItem::OnEventCallback, this);
    if (ret == 0) {
        events_ = events;
        is_inited_ = true;
        return true;
    }

    LogErr("event_assign() fail");
    return false;
}

void LibeventFdItem::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibeventFdItem::isEnabled() const
{
    if (!is_inited_)
        return false;

    short libevent_events = LocalEventsToLibevent(events_);
    return event_pending(&event_, libevent_events, NULL) != 0;
}

bool LibeventFdItem::enable()
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

bool LibeventFdItem::disable()
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

void LibeventFdItem::OnEventCallback(int /*fd*/, short events, void *args)
{
    LibeventFdItem *pthis = static_cast<LibeventFdItem*>(args);
    pthis->onEvent(events);
}

void LibeventFdItem::onEvent(short events)
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        short local_events = LibeventEventsToLocal(events);
        ++cb_level_;
        cb_(local_events);
        --cb_level_;
    } else {
        LogWarn("you should specify event callback by setCallback()");
    }

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop_->recordTimeCost(cost_us);
#endif
}

}
}
