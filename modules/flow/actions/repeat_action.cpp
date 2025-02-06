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
#include "repeat_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

RepeatAction::RepeatAction(event::Loop &loop)
  : SerialAssembleAction(loop, "Repeat")
{ }

RepeatAction::RepeatAction(event::Loop &loop, size_t times, Mode mode)
  : SerialAssembleAction(loop, "Repeat")
  , mode_(mode)
  , repeat_times_(times)
{ }

RepeatAction::RepeatAction(event::Loop &loop, Action *child, size_t times, Mode mode)
  : SerialAssembleAction(loop, "Repeat")
  , mode_(mode)
  , repeat_times_(times)
  , child_(child)
{
    TBOX_ASSERT(child_ != nullptr);
    bool is_set_parent_ok = child_->setParent(this);
    TBOX_ASSERT(is_set_parent_ok);
    UNUSED_VAR(is_set_parent_ok);

    child_->setFinishCallback(std::bind(&RepeatAction::onChildFinished, this, _1, _2, _3));
    child_->setBlockCallback(std::bind(&RepeatAction::block, this, _1, _2));
}

RepeatAction::~RepeatAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void RepeatAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);

    child_->toJson(js["child"]);
    js["repeat_times"] = repeat_times_;
    js["remain_times"] = remain_times_;
}

bool RepeatAction::setChild(Action *child) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    child->setFinishCallback(std::bind(&RepeatAction::onChildFinished, this, _1, _2, _3));
    child->setBlockCallback(std::bind(&RepeatAction::block, this, _1, _2));

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;

    return true;
}

bool RepeatAction::isReady() const {
    if (child_ == nullptr) {
        LogNotice("%d:%s[%s], no child", id(), type().c_str(), label().c_str());
        return false;
    }

    return child_->isReady();
}

void RepeatAction::onStart() {
    SerialAssembleAction::onStart();

    TBOX_ASSERT(child_ != nullptr);
    remain_times_ = repeat_times_ - 1;

    startThisAction(child_);
}

void RepeatAction::onReset() {
    TBOX_ASSERT(child_ != nullptr);
    child_->reset();

    SerialAssembleAction::onReset();
}

void RepeatAction::onChildFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (handleChildFinishEvent([this, is_succ, why, trace] { onChildFinished(is_succ, why, trace); }))
        return;

    if ((mode_ == Mode::kBreakSucc && is_succ) ||
        (mode_ == Mode::kBreakFail && !is_succ)) {
        finish(is_succ, why, trace);

    } else {
        if (remain_times_ > 0) {
            child_->reset();
            startThisAction(child_);
            --remain_times_;

        } else {
            finish(true, Reason(ACTION_REASON_REPEAT_NO_TIMES, "RepeatNoTimes"));
        }
    }
}

}
}
