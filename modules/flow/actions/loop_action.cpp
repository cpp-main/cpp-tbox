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
#include "loop_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

LoopAction::LoopAction(event::Loop &loop, Mode mode)
  : SerialAssembleAction(loop, "Loop")
  , mode_(mode)
{ }

LoopAction::LoopAction(event::Loop &loop, Action *child, Mode mode)
  : SerialAssembleAction(loop, "Loop")
  , child_(child)
  , mode_(mode)
{
    TBOX_ASSERT(child_ != nullptr);
    bool is_set_parent_ok = child_->setParent(this);
    TBOX_ASSERT(is_set_parent_ok);
    UNUSED_VAR(is_set_parent_ok);

    child_->setFinishCallback(std::bind(&LoopAction::onChildFinished, this, _1, _2, _3));
    child_->setBlockCallback(std::bind(&LoopAction::block, this, _1, _2));
}

LoopAction::~LoopAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void LoopAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);
    child_->toJson(js["child"]);
}

bool LoopAction::setChild(Action *child) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    child->setFinishCallback(std::bind(&LoopAction::onChildFinished, this, _1, _2, _3));
    child->setBlockCallback(std::bind(&LoopAction::block, this, _1, _2));

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;

    return true;
}

bool LoopAction::isReady() const {
    if (child_ == nullptr) {
        LogWarn("%d:%s[%s], child not set", id(), type().c_str(), label().c_str());
        return false;
    }
    return child_->isReady();
}

void LoopAction::onStart() {
    SerialAssembleAction::onStart();

    TBOX_ASSERT(child_ != nullptr);
    startThisAction(child_);
}

void LoopAction::onReset() {
    TBOX_ASSERT(child_ != nullptr);
    child_->reset();

    SerialAssembleAction::onReset();
}

void LoopAction::onChildFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (handleChildFinishEvent([this, is_succ, why, trace] { onChildFinished(is_succ, why, trace); }))
        return;

    if ((mode_ == Mode::kUntilSucc && is_succ) ||
        (mode_ == Mode::kUntilFail && !is_succ)) {
        finish(is_succ, why, trace);

    } else {
        child_->reset();
        startThisAction(child_);
    }
}

}
}
