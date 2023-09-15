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
#include "composite_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

CompositeAction::~CompositeAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void CompositeAction::setChild(Action *child) {
    TBOX_ASSERT(child != nullptr);

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;
    child_->setFinishCallback(std::bind(&CompositeAction::onChildFinished, this, _1));
}

void CompositeAction::toJson(Json &js) const {
    Action::toJson(js);
    child_->toJson(js["child"]);
}

bool CompositeAction::onStart() {
    if (child_ == nullptr) {
        LogWarn("no child in %d:%s(%s)", id(), type().c_str(), label().c_str());
        return false;
    }
    return child_->start();
}

bool CompositeAction::onStop() {
    return child_->stop();
}

bool CompositeAction::onPause() {
    return child_->pause();
}

bool CompositeAction::onResume() {
    if (child_->state() == State::kFinished) {
        finish(child_->result() == Result::kSuccess);
        return true;
    }
    return child_->resume();
}

void CompositeAction::onReset() {
    child_->reset();
}

void CompositeAction::onChildFinished(bool is_succ) {
    if (state() == State::kRunning)
        finish(is_succ);
}

}
}
