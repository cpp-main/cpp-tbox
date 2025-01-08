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
#include <tbox/base/wrapped_recorder.h>

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
        ++d_->read_event_num;

    if (events_ & kWriteEvent)
        ++d_->write_event_num;

    if (events_ & kExceptEvent)
        ++d_->except_event_num;

    d_->fd_events.push_back(this);

    reloadEpoll();

    is_enabled_ = true;
    return true;
}

bool EpollFdEvent::disable()
{
    if (d_ == nullptr || !is_enabled_)
        return true;

    if (events_ & kReadEvent)
        --d_->read_event_num;

    if (events_ & kWriteEvent)
        --d_->write_event_num;

    if (events_ & kExceptEvent)
        --d_->except_event_num;

    auto iter = std::find(d_->fd_events.begin(), d_->fd_events.end(), this);
    d_->fd_events.erase(iter);

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

    if (d_->write_event_num > 0)
        new_events |= EPOLLOUT;

    if (d_->read_event_num > 0)
        new_events |= EPOLLIN;

    if (d_->except_event_num > 0)
        new_events |= EPOLLERR;

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

void EpollFdEvent::OnEventCallback(uint32_t events, void *obj)
{
    RECORD_SCOPE();
    EpollFdSharedData *d = static_cast<EpollFdSharedData*>(obj);

    short tbox_events = 0;

    if (events & EPOLLIN) {
        events &= ~EPOLLIN;
        tbox_events |= kReadEvent;
    }

    if (events & EPOLLOUT) {
        events &= ~EPOLLOUT;
        tbox_events |= kWriteEvent;
    }

    if (events & EPOLLERR) {
        events &= ~EPOLLERR;
        tbox_events |= kExceptEvent;
    }

    if (events & EPOLLHUP) {
        events &= ~EPOLLHUP;
        //! 在epoll中，无论有没有监听EPOLLHUP，在对端close了fd时都会触发本端EPOLLHUP事件
        //! 只要发生了EPOLLHUB事件，只有让上层关闭该事件所有的事件才能停止EPOLLHUP的触发
        //! 否则它会一直触发事件，导致Loop空跑，CPU占满问题
        //! 为此，将HUP事件当成可读事件，上层读到0字节则表示对端已关闭
        tbox_events |= kReadEvent;
    }

    //! 要先复制一份，因为在for中很可能会改动到d->fd_events，引起迭代器失效问题
    auto tmp = d->fd_events;
    for (auto event : tmp)
        event->onEvent(tbox_events);

    if (events)
        LogWarn("unhandle events:%08X, fd:%d", events, d->fd);
}

void EpollFdEvent::onEvent(short events)
{
    if (events_ & events) {
        if (is_stop_after_trigger_)
            disable();

        wp_loop_->beginEventProcess();
        if (cb_) {
            RECORD_SCOPE();
            ++cb_level_;
            cb_(events);
            --cb_level_;
        }
        wp_loop_->endEventProcess(this);
    }
}

}
}
