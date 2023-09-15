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
#include "wrapper_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

WrapperAction::WrapperAction(event::Loop &loop, Action *child, Mode mode) :
    Action(loop, "Wrapper"),
    child_(child),
    mode_(mode)
{
    TBOX_ASSERT(child_ != nullptr);
    child_->setFinishCallback(std::bind(&WrapperAction::onChildFinished, this, _1));
}

WrapperAction::~WrapperAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void WrapperAction::toJson(Json &js) const {
    Action::toJson(js);
    js["mode"] = ToString(mode_);
    child_->toJson(js["child"]);
}

bool WrapperAction::onStart() {
    return child_->start();
}

bool WrapperAction::onStop() {
    return child_->stop();
}

bool WrapperAction::onPause() {
    return child_->pause();
}

bool WrapperAction::onResume() {
    if (child_->state() == State::kFinished) {
        onChildFinished(child_->result() == Result::kSuccess);
        return true;
    }
    return child_->resume();
}

void WrapperAction::onReset() {
    child_->reset();
}

void WrapperAction::onChildFinished(bool is_succ) {
    if (state() == State::kRunning) {
        if (mode_ == Mode::kNormal)
            finish(is_succ);
        else if (mode_ == Mode::kInvert)
            finish(!is_succ);
        else if (mode_ == Mode::kAlwaySucc)
            finish(true);
        else if (mode_ == Mode::kAlwayFail)
            finish(false);
        else
            TBOX_ASSERT(false);
    }
}

std::string ToString(WrapperAction::Mode mode) {
    const char *tbl[] = {"Normal", "Invert", "AlwaySucc", "AlwayFail"};
    auto idx = static_cast<size_t>(mode); // size_t id always >= 0
    if (idx < NUMBER_OF_ARRAY(tbl))
        return tbl[idx];
    else
        return "Unknown";
}

}
}
