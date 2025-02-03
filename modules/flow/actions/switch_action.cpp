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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "switch_action.h"

#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>
#include <tbox/util/string.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

SwitchAction::SwitchAction(event::Loop &loop)
    : AssembleAction(loop, "Switch")
{ }

SwitchAction::~SwitchAction() {
    CHECK_DELETE_RESET_OBJ(switch_action_);
    CHECK_DELETE_RESET_OBJ(default_action_);
    for (auto &item : case_actions_)
        CHECK_DELETE_OBJ(item.second);
    case_actions_.clear();
}

void SwitchAction::toJson(Json &js) const {
    AssembleAction::toJson(js);

    auto &js_children = js["children"];
    switch_action_->toJson(js_children["00.switch"]);

    if (default_action_ != nullptr)
        default_action_->toJson(js_children["99.default"]);

    int index = 1;
    for (auto &item : case_actions_) {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << index << '.' << item.first;
        item.second->toJson(js_children[oss.str()]);
        ++index;
    }
}

bool SwitchAction::setChildAs(Action *child, const std::string &role) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], child is nullptr, role:%s", id(), type().c_str(), label().c_str(), role.c_str());
        return false;
    }

    if (role == "switch") {
        child->setFinishCallback(std::bind(&SwitchAction::onSwitchActionFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&SwitchAction::block, this, _1, _2));
        child->setParent(this);
        CHECK_DELETE_RESET_OBJ(switch_action_);
        switch_action_ = child;
        return true;

    } else if (role == "default") {
        child->setFinishCallback(std::bind(&SwitchAction::finish, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&SwitchAction::block, this, _1, _2));
        child->setParent(this);
        CHECK_DELETE_RESET_OBJ(default_action_);
        default_action_ = child;
        return true;

    } else if (util::string::IsStartWith(role, "case:")) {
        auto result = case_actions_.emplace(role, child);
        if (result.second) {
            child->setFinishCallback(std::bind(&SwitchAction::finish, this, _1, _2, _3));
            child->setBlockCallback(std::bind(&SwitchAction::block, this, _1, _2));
            child->setParent(this);
            return true;
        }
    }

    LogWarn("%d:%s[%s], unsupport role:%s", id(), type().c_str(), label().c_str(), role.c_str());
    return false;
}

bool SwitchAction::isReady() const {
    //! switch_action没有安装，自然就没有就绪
    if (switch_action_ == nullptr) {
        LogWarn("%d:%s[%s] no switch action", id(), type().c_str(), label().c_str());
        return false;
    }

    //! 如果default与case都没有，则表示Action没有安装好
    if (!default_action_ && case_actions_.empty()) {
        LogWarn("%d:%s[%s] both default and case action is null", id(), type().c_str(), label().c_str());
        return false;
    }

    if (!switch_action_->isReady())
        return false;

    if (default_action_ != nullptr && !default_action_->isReady())
        return false;

    //! 检查所有的case_action，看看有没有哪个没有就绪
    for (auto &item : case_actions_) {
        if (!item.second->isReady())
            return false;
    }

    return true;
}

void SwitchAction::onStart() {
    AssembleAction::onStart();

    TBOX_ASSERT(switch_action_ != nullptr);
    running_action_ = switch_action_;
    running_action_->start();
}

void SwitchAction::onStop() {
    if (running_action_ != nullptr)
        running_action_->stop();

    AssembleAction::onStop();
}

void SwitchAction::onPause() {
    if (running_action_ != nullptr)
        running_action_->pause();

    AssembleAction::onPause();
}

void SwitchAction::onResume() {
    AssembleAction::onResume();

    if (running_action_ != nullptr)
        running_action_->resume();
}

void SwitchAction::onReset() {
    TBOX_ASSERT(switch_action_ != nullptr);
    switch_action_->reset();

    if (default_action_ != nullptr)
        default_action_->reset();

    for (auto &item : case_actions_)
        item.second->reset();

    running_action_ = nullptr;

    AssembleAction::onReset();
}

void SwitchAction::onSwitchActionFinished(bool is_succ, const Reason &why, const Trace &) {
    if (state() == State::kRunning) {
        if (is_succ) {
            running_action_ = default_action_;

            auto iter = case_actions_.find(why.message);
            if (iter != case_actions_.end())
                running_action_ = iter->second;

            if (running_action_ != nullptr) {
                running_action_->start();
            } else {
                finish(false, Reason(ACTION_REASON_SWITCH_SKIP, "SwitchSkip"));
            }

        } else {
            finish(false, Reason(ACTION_REASON_SWITCH_FAIL, "SwitchFail"));
        }
    }
}

}
}
