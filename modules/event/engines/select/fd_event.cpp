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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <algorithm>

#include "fd_event.h"
#include "loop.h"
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

namespace tbox {
namespace event {

SelectFdEvent::SelectFdEvent(SelectLoop *wp_loop, const std::string &what)
  : FdEvent(what)
  , wp_loop_(wp_loop)
{ }

SelectFdEvent::~SelectFdEvent()
{
    TBOX_ASSERT(cb_level_ == 0);

    disable();

    wp_loop_->unrefFdSharedData(fd_);
}

bool SelectFdEvent::initialize(int fd, short events, Mode mode)
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

bool SelectFdEvent::enable()
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

    is_enabled_ = true;
    return true;
}

bool SelectFdEvent::disable()
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

    is_enabled_ = false;
    return true;
}

Loop* SelectFdEvent::getLoop() const
{
    return wp_loop_;
}

void SelectFdEvent::OnEventCallback(bool is_readable, bool is_writable, bool is_except, SelectFdSharedData *data)
{
    RECORD_SCOPE();
    short tbox_events = 0;

    if (is_readable)
        tbox_events |= kReadEvent;

    if (is_writable)
        tbox_events |= kWriteEvent;

    if (is_except)
        tbox_events |= kExceptEvent;

    //! 要先复制一份，因为在for中很可能会改动到d->fd_events，引起迭代器失效问题
    auto tmp = data->fd_events;
    for (auto event : tmp)
        event->onEvent(tbox_events);
}

void SelectFdEvent::onEvent(short events)
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
