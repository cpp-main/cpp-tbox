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
#include "if_else_action.h"

#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

IfElseAction::IfElseAction(event::Loop &loop)
    : SerialAssembleAction(loop, "IfElse")
{ }

IfElseAction::~IfElseAction() {
    CHECK_DELETE_RESET_OBJ(if_action_);
    CHECK_DELETE_RESET_OBJ(then_action_);
    CHECK_DELETE_RESET_OBJ(else_action_);
}

void IfElseAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);
    auto &js_children = js["children"];
    if_action_->toJson(js_children["0.if"]);
    if (then_action_ != nullptr)
        then_action_->toJson(js_children["1.then"]);
    if (else_action_ != nullptr)
        else_action_->toJson(js_children["2.else"]);
}

bool IfElseAction::setChildAs(Action *child, const std::string &role) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    if (role == "if") {
        child->setFinishCallback(std::bind(&IfElseAction::onCondActionFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        CHECK_DELETE_RESET_OBJ(if_action_);
        if_action_ = child;
        return true;

    } else if (role == "succ" || role == "then") {
        child->setFinishCallback(std::bind(&IfElseAction::onLastChildFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        CHECK_DELETE_RESET_OBJ(then_action_);
        then_action_ = child;
        return true;

    } else if (role == "fail" || role == "else") {
        child->setFinishCallback(std::bind(&IfElseAction::onLastChildFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        CHECK_DELETE_RESET_OBJ(else_action_);
        else_action_ = child;
        return true;
    }

    child->setParent(nullptr);
    LogWarn("%d:%s[%s], unsupport role:%s", id(), type().c_str(), label().c_str(), role.c_str());
    return false;
}

bool IfElseAction::isReady() const {
    if (if_action_ == nullptr) {
        LogWarn("%d:%s[%s] no cond func", id(), type().c_str(), label().c_str());
        return false;
    }

    if (!then_action_ && !else_action_) {
        LogWarn("%d:%s[%s] both succ and fail func is null", id(), type().c_str(), label().c_str());
        return false;
    }

    if ((!if_action_->isReady()) ||
        (then_action_ != nullptr && !then_action_->isReady()) ||
        (else_action_ != nullptr && !else_action_->isReady()))
        return false;

    return true;
}

void IfElseAction::onStart() {
    SerialAssembleAction::onStart();

    startThisAction(if_action_);
}

void IfElseAction::onReset() {
    TBOX_ASSERT(if_action_ != nullptr);
    if_action_->reset();

    if (then_action_ != nullptr)
        then_action_->reset();

    if (else_action_ != nullptr)
        else_action_->reset();

    SerialAssembleAction::onReset();
}

void IfElseAction::onCondActionFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (handleChildFinishEvent([this, is_succ, why, trace] { onCondActionFinished(is_succ, why, trace); }))
        return;

    if (is_succ) {
        if (then_action_ != nullptr) {
            startThisAction(then_action_);
            return;
        }
    } else {
        if (else_action_ != nullptr) {
            startThisAction(else_action_);
            return;
        }
    }
    finish(true, why, trace);
}

}
}
