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
#include "loop_if_action.h"

#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

LoopIfAction::LoopIfAction(event::Loop &loop)
  : SerialAssembleAction(loop, "LoopIf")
{ }

LoopIfAction::~LoopIfAction() {
  CHECK_DELETE_RESET_OBJ(if_action_);
  CHECK_DELETE_RESET_OBJ(exec_action_);
}

void LoopIfAction::toJson(Json &js) const {
  SerialAssembleAction::toJson(js);
  auto &js_children = js["children"];
  if_action_->toJson(js_children["0.if"]);
  exec_action_->toJson(js_children["1.exec"]);
}

bool LoopIfAction::setChildAs(Action *child, const std::string &role) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    if (role == "if") {
        child->setFinishCallback(std::bind(&LoopIfAction::onIfFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&LoopIfAction::block, this, _1, _2));
        CHECK_DELETE_RESET_OBJ(if_action_);
        if_action_ = child;
        return true;

    } else if (role == "exec") {
        child->setFinishCallback(std::bind(&LoopIfAction::onExecFinished, this, _2, _3));
        child->setBlockCallback(std::bind(&LoopIfAction::block, this, _1, _2));
        CHECK_DELETE_RESET_OBJ(exec_action_);
        exec_action_ = child;
        return true;
    }

    child->setParent(nullptr);
    LogWarn("%d:%s[%s], unsupport role:%s", id(), type().c_str(), label().c_str(), role.c_str());

    return false;
}

bool LoopIfAction::isReady() const {
    if (if_action_ == nullptr) {
        LogWarn("%d:%s[%s], if_action == nullptr", id(), type().c_str(), label().c_str());
        return false;
    }

    if (exec_action_ == nullptr) {
        LogWarn("%d:%s[%s], exec_action == nullptr", id(), type().c_str(), label().c_str());
        return false;
    }

    if (!if_action_->isReady() || !exec_action_->isReady())
        return false;

    return true;
}

void LoopIfAction::onStart() {
    SerialAssembleAction::onStart();

    TBOX_ASSERT(if_action_ != nullptr);
    startThisAction(if_action_);
}

void LoopIfAction::onReset() {
    TBOX_ASSERT(if_action_ != nullptr && exec_action_ != nullptr);
    if_action_->reset();
    exec_action_->reset();

    SerialAssembleAction::onReset();
}

void LoopIfAction::onIfFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (handleChildFinishEvent([this, is_succ, why, trace] { onIfFinished(is_succ, why, trace); }))
        return;

    if (is_succ) {
        startThisAction(exec_action_);
    } else {
        finish(finish_result_, why, trace);
    }
}

void LoopIfAction::onExecFinished(const Reason &why, const Trace &trace) {
    if (handleChildFinishEvent([this, why, trace] { onExecFinished(why, trace); }))
        return;

    if_action_->reset();
    exec_action_->reset();

    if (!startThisAction(if_action_))
        finish(finish_result_, why, trace);
}

}
}
