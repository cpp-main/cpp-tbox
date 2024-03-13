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

bool CompositeAction::setChild(Action *child) {
    if (child == nullptr)
        return false;

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;
    child_->setFinishCallback(std::bind(&CompositeAction::onChildFinished, this, _1));
    child_->setBlockCallback(std::bind(&CompositeAction::onChildBlocked, this, _1));

    return true;
}

void CompositeAction::toJson(Json &js) const {
    TBOX_ASSERT(child_ != nullptr);
    AssembleAction::toJson(js);
    child_->toJson(js["child"]);
}

bool CompositeAction::isReady() const {
    if (child_ != nullptr)
        return child_->isReady();

    LogWarn("%d:%s[%s] child not set", id(), type().c_str(), label().c_str());
    return false;
}

void CompositeAction::onStart() {
    AssembleAction::onStart();

    TBOX_ASSERT(child_ != nullptr);
    child_->start();
}

void CompositeAction::onStop() {
    TBOX_ASSERT(child_ != nullptr);
    child_->stop();

    AssembleAction::onStop();
}

void CompositeAction::onPause() {
    TBOX_ASSERT(child_ != nullptr);
    child_->pause();

    AssembleAction::onPause();
}

void CompositeAction::onResume() {
    AssembleAction::onResume();

    TBOX_ASSERT(child_ != nullptr);
    if (child_->state() == State::kFinished) {
        finish(child_->result() == Result::kSuccess);
    } else {
        child_->resume();
    }
}

void CompositeAction::onReset() {
    TBOX_ASSERT(child_ != nullptr);
    child_->reset();

    AssembleAction::onReset();
}

void CompositeAction::onFinished(bool is_succ) {
    //! 有可能不是child_自然结束产生的finish
    TBOX_ASSERT(child_ != nullptr);
    child_->stop();

    AssembleAction::onFinished(is_succ);
}

void CompositeAction::onChildFinished(bool is_succ) {
    if (state() == State::kRunning)
        finish(is_succ);
}

void CompositeAction::onChildBlocked(int why) {
    if (state() == State::kRunning)
        block(why);
}

}
}
