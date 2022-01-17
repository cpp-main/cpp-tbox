#include <cassert>
#include <cstring>
#include <sys/epoll.h>
#include "fd_event.h"
#include "loop.h"

namespace tbox {
namespace event {

EpollFdEvent::EpollFdEvent(EpollLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    is_stop_after_trigger_(false),
    cb_level_(0)
{

}

EpollFdEvent::~EpollFdEvent()
{
    assert(cb_level_ == 0);
    disable();
}

namespace {
short EpollEventsToLocal(short epoll_events)
{
    short ret = 0;
    if ((epoll_events & EPOLLIN) || (epoll_events & EPOLLPRI))
        ret |= FdEvent::kReadEvent;
    if (epoll_events & EPOLLOUT)
        ret |= FdEvent::kWriteEvent;

    return ret;
}

short LocalEventsToEpoll(short local_events)
{
    short ret = 0;
    if (local_events & FdEvent::kWriteEvent)
        ret |= EPOLLOUT;

    if (local_events & FdEvent::kReadEvent)
        ret |= EPOLLIN;

    return ret;
}

}

bool EpollFdEvent::initialize(int fd, short events, Mode mode)
{
    disable();
    uint32_t epoll_events = LocalEventsToEpoll(events);
    if (!fd_event_data_)
        fd_event_data_ = new EventData(fd, this, EpollFdEvent::HandleEvent, epoll_events);


    if (mode == Mode::kOneshot)
        is_stop_after_trigger_ = true;

    is_inited_ = true;
    return true;
}

void EpollFdEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool EpollFdEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return is_enabled_;
}

bool EpollFdEvent::enable()
{
    if (!is_inited_)
        return false;

    if (isEnabled())
        return true;

    if (!fd_event_data_)
        return false;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = fd_event_data_->events;
    ev.data.ptr = static_cast<void *>(fd_event_data_);

    int ret = epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_ADD, fd_event_data_->fd, &ev);
    if (ret != 0)
        return false;

    is_enabled_ = true;
    return true;
}

bool EpollFdEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    if (fd_event_data_) {
        epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_DEL, fd_event_data_->fd, NULL);
        delete fd_event_data_;
        fd_event_data_ = nullptr;
    }

    is_enabled_ = false;
    return true;
}

Loop* EpollFdEvent::getLoop() const
{
    return wp_loop_;
}

void EpollFdEvent::HandleEvent(int fd, uint32_t events, void *obj)
{
    EpollFdEvent *self = reinterpret_cast<EpollFdEvent*>(obj);

    if (!self)
        return;

    self->onEvent(events);
}

void EpollFdEvent::onEvent(short events)
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        short local_events = EpollEventsToLocal(events);

        ++cb_level_;
        cb_(local_events);
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();
    }

    auto wp_loop = wp_loop_;
    wp_loop->handleNextFunc();

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop->recordTimeCost(cost_us);
#endif
}

}
}
