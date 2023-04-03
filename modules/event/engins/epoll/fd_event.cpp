#include <cstring>
#include <algorithm>
#include <vector>

#include "fd_event.h"
#include "loop.h"
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace event {

//! 同一个fd共享的数据
struct EpollFdSharedData {
    int ref = 0;    //! 引用计数
    struct epoll_event ev;
    std::vector<EpollFdEvent*> read_events;
    std::vector<EpollFdEvent*> write_events;
};

EpollFdEvent::EpollFdEvent(EpollLoop *wp_loop) :
    wp_loop_(wp_loop)
{ }

EpollFdEvent::~EpollFdEvent()
{
    TBOX_ASSERT(cb_level_ == 0);

    disable();

    unrefFdSharedData();
}

bool EpollFdEvent::initialize(int fd, short events, Mode mode)
{
    if (isEnabled())
        return false;

    unrefFdSharedData();

    fd_ = fd;
    events_ = events;
    if (mode == FdEvent::Mode::kOneshot)
        is_stop_after_trigger_ = true;

    d_ = wp_loop_->queryFdSharedData(fd_);
    if (d_ == nullptr) {
        d_ = new EpollFdSharedData; 
        TBOX_ASSERT(d_ != nullptr);

        memset(&d_->ev, 0, sizeof(d_->ev));
        d_->ev.data.ptr = static_cast<void *>(d_);

        wp_loop_->addFdSharedData(fd_, d_);
    }

    ++d_->ref;
    return true;
}

bool EpollFdEvent::enable()
{
    if (d_ == nullptr)
        return false;

    if (is_enabled_)
        return true;

    if (events_ & kReadEvent)
        d_->read_events.push_back(this);

    if (events_ & kWriteEvent)
        d_->write_events.push_back(this);

    reloadEpoll();

    is_enabled_ = true;
    return true;
}

bool EpollFdEvent::disable()
{
    if (d_ == nullptr || !is_enabled_)
        return true;

    if (events_ & kReadEvent) {
        auto iter = std::find(d_->read_events.begin(), d_->read_events.end(), this);
        d_->read_events.erase(iter);
    }

    if (events_ & kWriteEvent) {
        auto iter = std::find(d_->write_events.begin(), d_->write_events.end(), this);
        d_->write_events.erase(iter);
    }

    reloadEpoll();

    is_enabled_ = false;
    return true;
}

Loop* EpollFdEvent::getLoop() const
{
    return wp_loop_;
}

//! 重新加载fd对应的epoll
void EpollFdEvent::reloadEpoll()
{
    epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_DEL, fd_, NULL);

    d_->ev.events = 0;
    if (!d_->write_events.empty())
        d_->ev.events |= EPOLLOUT;
    if (!d_->read_events.empty())
        d_->ev.events |= EPOLLIN;

    epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_ADD, fd_, &d_->ev);
}

void EpollFdEvent::OnEventCallback(int fd, uint32_t events, void *obj)
{
    EpollFdSharedData *d = static_cast<EpollFdSharedData*>(obj);

    if (events & EPOLLIN) {
        for (EpollFdEvent *event : d->read_events)
            event->onEvent(kReadEvent);
    }

    if (events & EPOLLOUT) {
        for (EpollFdEvent *event : d->write_events)
            event->onEvent(kWriteEvent);
    }
}

void EpollFdEvent::onEvent(short events)
{
    wp_loop_->beginEventProcess();

    if (is_stop_after_trigger_)
        disable();

    if (cb_) {
        ++cb_level_;
        cb_(events);
        --cb_level_;
    }

    wp_loop_->endEventProcess();
}

void EpollFdEvent::unrefFdSharedData()
{
    if (d_ != nullptr) {
        --d_->ref;
        if (d_->ref == 0) {
            wp_loop_->removeFdSharedData(fd_);
            delete d_;
            d_ = nullptr;
            fd_ = -1;
        }
    }
}

}
}
