#include <cassert>
#include <cstring>
#include "fd_event.h"
#include "loop.h"

namespace tbox::event {

EpollFdEvent::EpollFdEvent(BuiltinLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    is_stop_after_trigger_(false),
    cb_level_(0)
{
    memset(&ev_, 0, sizeof(ev_));
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

    fd_ = fd;
    memset(&ev_, 0, sizeof(ev_));
    ev_.data.ptr = static_cast<void *>(this);
    ev_.events = LocalEventsToEpoll(events);

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

    int ret = epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_ADD, fd_, &ev_);
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

    epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_DEL, fd_, NULL);

    is_enabled_ = false;
    return true;
}

Loop* EpollFdEvent::getLoop() const
{
    return wp_loop_;
}

void EpollFdEvent::OnEventCallback(int fd, uint32_t events, void *obj)
{
    EpollFdEvent *pthis = static_cast<EpollFdEvent*>(obj);
    pthis->onEvent(events);
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
