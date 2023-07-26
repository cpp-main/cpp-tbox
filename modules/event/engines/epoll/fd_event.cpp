/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <algorithm>
#include <vector>

#include "fd_event.h"
#include "loop.h"
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>

namespace tbox {
namespace event {

EpollFdEvent::EpollFdEvent(EpollLoop *wp_loop, const std::string &what)
  : FdEvent(what)
  , wp_loop_(wp_loop)
{ }

EpollFdEvent::~EpollFdEvent()
{
    TBOX_ASSERT(cb_level_ == 0);

    disable();

    wp_loop_->unrefFdSharedData(fd_);
}

bool EpollFdEvent::initialize(int fd, short events, Mode mode)
{
    if (isEnabled())
        return false;

    if (fd != fd_) {
        wp_loop_->unrefFdSharedData(fd_);
        fd_ = fd;
        d_ = wp_loop_->refFdSharedData(fd_);
    }

    events_ = events;
    if (mode == FdEvent::Mode::kOneshot)
        is_stop_after_trigger_ = true;

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

    if (events_ & kExceptEvent)
        d_->exception_events.push_back(this);

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

    if (events_ & kExceptEvent) {
        auto iter = std::find(d_->exception_events.begin(), d_->exception_events.end(), this);
        d_->exception_events.erase(iter);
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
    uint32_t old_events = d_->ev.events;
    uint32_t new_events = 0;

    if (!d_->write_events.empty())
        new_events |= EPOLLOUT;
    if (!d_->read_events.empty())
        new_events |= EPOLLIN;
    if (!d_->exception_events.empty())
        new_events |= (EPOLLHUP | EPOLLERR);

    d_->ev.events = new_events;

    if (old_events == 0) {
        if (LIKELY(new_events != 0))
            epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_ADD, fd_, &d_->ev);
    } else {
        if (new_events != 0)
            epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_MOD, fd_, &d_->ev);
        else
            epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_DEL, fd_, nullptr);
    }
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

    if (events & EPOLLHUP || events & EPOLLERR) {
        for (EpollFdEvent *event : d->exception_events)
            event->onEvent(kExceptEvent);
    }

    (void)fd;
}

void EpollFdEvent::onEvent(short events)
{
    if (is_stop_after_trigger_)
        disable();

    wp_loop_->beginEventProcess();
    if (cb_) {
        ++cb_level_;
        cb_(events);
        --cb_level_;
    }
    wp_loop_->endEventProcess(this);
}

}
}
