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
    : AssembleAction(loop, "IfElse")
{ }

IfElseAction::~IfElseAction() {
    CHECK_DELETE_RESET_OBJ(if_action_);
    CHECK_DELETE_RESET_OBJ(succ_action_);
    CHECK_DELETE_RESET_OBJ(fail_action_);
}

void IfElseAction::toJson(Json &js) const {
    AssembleAction::toJson(js);
    auto &js_children = js["children"];
    if_action_->toJson(js_children["0.if"]);
    if (succ_action_ != nullptr)
        succ_action_->toJson(js_children["1.succ"]);
    if (fail_action_ != nullptr)
        fail_action_->toJson(js_children["2.fail"]);
}

bool IfElseAction::setChildAs(Action *child, const std::string &role) {
    if (role == "if") {
        CHECK_DELETE_RESET_OBJ(if_action_);
        if_action_ = child;
        if (if_action_ != nullptr) {
            if_action_->setFinishCallback(std::bind(&IfElseAction::onCondActionFinished, this, _1, _2, _3));
            if_action_->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        }
        return true;

    } else if (role == "succ") {
        CHECK_DELETE_RESET_OBJ(succ_action_);
        succ_action_ = child;
        if (succ_action_ != nullptr) {
            succ_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1, _2, _3));
            succ_action_->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        }
        return true;

    } else if (role == "fail") {
        CHECK_DELETE_RESET_OBJ(fail_action_);
        fail_action_ = child;
        if (fail_action_ != nullptr) {
            fail_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1, _2, _3));
            fail_action_->setBlockCallback(std::bind(&IfElseAction::block, this, _1, _2));
        }
        return true;
    }

    LogWarn("%d:%s[%s], unsupport role:%s", id(), type().c_str(), label().c_str(), role.c_str());
    return false;
}

bool IfElseAction::isReady() const {
    if (if_action_ == nullptr) {
        LogWarn("%d:%s[%s] no cond func", id(), type().c_str(), label().c_str());
        return false;
    }

    if (!succ_action_ && !fail_action_) {
        LogWarn("%d:%s[%s] both succ and fail func is null", id(), type().c_str(), label().c_str());
        return false;
    }

    if ((!if_action_->isReady()) ||
        (succ_action_ != nullptr && !succ_action_->isReady()) ||
        (fail_action_ != nullptr && !fail_action_->isReady()))
        return false;

    return true;
}

void IfElseAction::onStart() {
    AssembleAction::onStart();

    TBOX_ASSERT(if_action_ != nullptr);
    if_action_->start();
}

void IfElseAction::onStop() {
    TBOX_ASSERT(if_action_ != nullptr);
    if (if_action_->state() == State::kFinished) {
        if (if_action_->result() == Result::kSuccess) {
            succ_action_->stop();
        } else {
            fail_action_->stop();
        }
    } else {
        if_action_->stop();
    }

    AssembleAction::onStop();
}

void IfElseAction::onPause() {
    TBOX_ASSERT(if_action_ != nullptr);
    if (if_action_->state() == State::kFinished) {
        if (if_action_->result() == Result::kSuccess) {
            succ_action_->pause();
        } else {
            fail_action_->pause();
        }
    } else {
        if_action_->pause();
    }

    AssembleAction::onPause();
}

void IfElseAction::onResume() {
    AssembleAction::onResume();

    TBOX_ASSERT(if_action_ != nullptr);
    if (if_action_->state() == State::kFinished) {
        if (if_action_->result() == Result::kSuccess) {
            if (succ_action_->state() == State::kFinished) {
                finish(succ_action_->result() == Result::kSuccess);
            } else {
                succ_action_->resume();
            }
        } else {
            if (fail_action_->state() == State::kFinished) {
                finish(fail_action_->result() == Result::kSuccess);
            } else {
                fail_action_->resume();
            }
        }
    } else {
        if_action_->resume();
    }
}

void IfElseAction::onReset() {
    TBOX_ASSERT(if_action_ != nullptr);
    if_action_->reset();

    if (succ_action_ != nullptr)
        succ_action_->reset();

    if (fail_action_ != nullptr)
        fail_action_->reset();

    AssembleAction::onReset();
}

void IfElseAction::onCondActionFinished(bool is_succ, const Reason &why, const Trace &trace) {
    if (state() == State::kRunning) {
        if (is_succ) {
            if (succ_action_ != nullptr) {
                succ_action_->start();
                return;
            }
        } else {
            if (fail_action_ != nullptr) {
                fail_action_->start();
                return;
            }
        }
        finish(true, why, trace);
    }
}

}
}
