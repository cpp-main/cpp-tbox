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
#include "signal_event_impl.h"
#include "common_loop.h"
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>

namespace tbox {
namespace event {

SignalEventImpl::SignalEventImpl(CommonLoop *wp_loop, const std::string &what)
    : SignalEvent(what)
    , wp_loop_(wp_loop)
{ }

SignalEventImpl::~SignalEventImpl()
{
    TBOX_ASSERT(cb_level_ == 0);
    disable();
}

bool SignalEventImpl::initialize(int signo, Mode mode)
{
    sigset_.insert(signo);
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::initialize(const std::set<int> &sigset, Mode mode)
{
    sigset_ = sigset;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::initialize(const std::initializer_list<int> &sigset, Mode mode)
{
    for (auto signo : sigset)
        sigset_.insert(signo);

    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::enable()
{
    if (is_inited_) {
        for (int signo : sigset_) {
            if (!wp_loop_->subscribeSignal(signo, this)) {
                return false;
            }
        }
    }
    is_enabled_ = true;
    return true;
}

bool SignalEventImpl::disable()
{
    if (is_enabled_) {
        for (int signo : sigset_) {
            if (!wp_loop_->unsubscribeSignal(signo, this)) {
                return false;
            }
        }
    }
    is_enabled_ = false;
    return true;
}

Loop* SignalEventImpl::getLoop() const
{
    return wp_loop_;
}

void SignalEventImpl::onSignal(int signo)
{
    if (mode_ == Mode::kOneshot)
        disable();

    wp_loop_->beginEventProcess();
    if (cb_) {
        RECORD_SCOPE();
        ++cb_level_;
        cb_(signo);
        --cb_level_;
    }
    wp_loop_->endEventProcess(this);
}

}
}
