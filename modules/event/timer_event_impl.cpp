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
#include <tbox/base/assert.h>

#include "common_loop.h"
#include "timer_event_impl.h"

namespace tbox {
namespace event {

TimerEventImpl::TimerEventImpl(CommonLoop *wp_loop, const std::string &what)
    : TimerEvent(what)
    , wp_loop_(wp_loop)
{ }

TimerEventImpl::~TimerEventImpl()
{
    TBOX_ASSERT(cb_level_ == 0);
    disable();
}

bool TimerEventImpl::initialize(const std::chrono::milliseconds &interval, Mode mode)
{
    disable();

    interval_ = interval;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool TimerEventImpl::isEnabled() const
{
    if (!is_inited_)
        return false;

    return is_enabled_;
}

bool TimerEventImpl::enable()
{
    if (!is_inited_)
        return false;

    if (isEnabled())
        return true;

    if (wp_loop_)
        token_ = wp_loop_->addTimer(interval_.count(), mode_ == Mode::kOneshot ? 1 : 0, [this]{ onEvent(); });

    is_enabled_ = true;

    return true;
}

bool TimerEventImpl::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    if (wp_loop_)
        wp_loop_->deleteTimer(token_);

    is_enabled_ = false;
    return true;
}

Loop* TimerEventImpl::getLoop() const
{
    return wp_loop_;
}

void TimerEventImpl::onEvent()
{
    if (mode_ == Mode::kOneshot) {
        is_enabled_ = false;
        token_.reset();
    }

    wp_loop_->beginEventProcess();
    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;
    }
    wp_loop_->endEventProcess(this);
}

}
}
