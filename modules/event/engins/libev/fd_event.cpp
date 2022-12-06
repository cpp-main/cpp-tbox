#include "fd_event.h"

#include "loop.h"
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace event {

LibevFdEvent::LibevFdEvent(LibevLoop *wp_loop) :
    wp_loop_(wp_loop)
{
    memset(&io_ev_, 0, sizeof(io_ev_));
}

LibevFdEvent::~LibevFdEvent()
{
    TBOX_ASSERT(cb_level_ == 0);
    disable();
}

namespace {
short LibevEventsToLocal(short libev_events)
{
    short ret = 0;
    if (libev_events & EV_READ)
        ret |= FdEvent::kReadEvent;
    if (libev_events & EV_WRITE)
        ret |= FdEvent::kWriteEvent;

    return ret;
}

short LocalEventsToLibev(short local_events)
{
    short ret = 0;
    if (local_events & FdEvent::kWriteEvent)
        ret |= EV_WRITE;
    if (local_events & FdEvent::kReadEvent)
        ret |= EV_READ;

    return ret;
}

}

bool LibevFdEvent::initialize(int fd, short events, Mode mode)
{
    disable();

    short libev_events = LocalEventsToLibev(events);
    io_ev_.active = io_ev_.pending = 0;
    io_ev_.priority = 0;
    io_ev_.cb = LibevFdEvent::OnEventCallback;
    io_ev_.data = this;

    io_ev_.fd = fd;
    io_ev_.events = libev_events | EV__IOFDSET;

    if (mode == Mode::kOneshot) //! 如果是单次有效的，需要设置标记，使之在触发后停止事件
        is_stop_after_trigger_ = true;

    is_inited_ = true;
    return true;
}

void LibevFdEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibevFdEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return io_ev_.active;
}

bool LibevFdEvent::enable()
{
    if (!is_inited_) {
        //! 没有初始化，是不能直接enable的
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    ev_io_start(wp_loop_->getEvLoopPtr(), &io_ev_);

    return true;
}

bool LibevFdEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    ev_io_stop(wp_loop_->getEvLoopPtr(), &io_ev_);

    return true;
}

Loop* LibevFdEvent::getLoop() const
{
    return wp_loop_;
}

void LibevFdEvent::OnEventCallback(struct ev_loop*, ev_io *p_w, int events)
{
    TBOX_ASSERT(p_w != NULL);

    LibevFdEvent *pthis = static_cast<LibevFdEvent*>(p_w->data);
    pthis->onEvent(events);
}

void LibevFdEvent::onEvent(short events)
{
    wp_loop_->beginEventProcess();

    if (cb_) {
        short local_events = LibevEventsToLocal(events);

        ++cb_level_;
        cb_(local_events);
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();

    } else {
        LogWarn("WARN: you should specify event callback by setCallback()");
    }

    wp_loop_->endEventProcess();
}

}
}
