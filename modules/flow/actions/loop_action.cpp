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
  : AssembleAction(loop, "Loop")
  , mode_(mode)
{ }

LoopAction::LoopAction(event::Loop &loop, Action *child, Mode mode)
  : AssembleAction(loop, "Loop")
  , child_(child)
  , mode_(mode)
{
    TBOX_ASSERT(child_ != nullptr);
    child_->setFinishCallback(std::bind(&LoopAction::onChildFinished, this, _1, _2, _3));
    child_->setBlockCallback(std::bind(&LoopAction::block, this, _1, _2));
}

LoopAction::~LoopAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void LoopAction::toJson(Json &js) const {
    AssembleAction::toJson(js);
    child_->toJson(js["child"]);
}

bool LoopAction::setChild(Action *child) {
    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;
    if (child_ != nullptr) {
        child_->setFinishCallback(std::bind(&LoopAction::onChildFinished, this, _1, _2, _3));
        child_->setBlockCallback(std::bind(&LoopAction::block, this, _1, _2));
    }
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
    AssembleAction::onStart();

    TBOX_ASSERT(child_ != nullptr);
    child_->start();
}

void LoopAction::onStop() {
    TBOX_ASSERT(child_ != nullptr);
    child_->stop();

    AssembleAction::onStop();
}

void LoopAction::onPause() {
    TBOX_ASSERT(child_ != nullptr);
    child_->pause();

    AssembleAction::onPause();
}

void LoopAction::onResume() {
    AssembleAction::onResume();

    TBOX_ASSERT(child_ != nullptr);
    child_->resume();
}

void LoopAction::onReset() {
    TBOX_ASSERT(child_ != nullptr);
    child_->reset();

    AssembleAction::onReset();
}

void LoopAction::onChildFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (state() == State::kRunning) {
        if ((mode_ == Mode::kUntilSucc && is_succ) ||
            (mode_ == Mode::kUntilFail && !is_succ)) {
            finish(is_succ, why, trace);
        } else if (state() == State::kRunning) {
            child_->reset();
            child_->start();
        }
    }
}

}
}
