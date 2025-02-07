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

#include "if_then_action.h"

#include <sstream>
#include <iomanip>

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

IfThenAction::IfThenAction(event::Loop &loop)
    : SerialAssembleAction(loop, "IfThen")
{ }

IfThenAction::~IfThenAction() {
    CHECK_DELETE_RESET_OBJ(tmp_.first);
    CHECK_DELETE_RESET_OBJ(tmp_.second);

    for (auto &item : if_then_actions_) {
        CHECK_DELETE_OBJ(item.first);
        CHECK_DELETE_OBJ(item.second);
    }
    if_then_actions_.clear();
}

void IfThenAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);

    auto &js_children = js["children"];

    int prefix_num = 0;
    for (auto &item : if_then_actions_) {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << prefix_num << '.';
        auto prefix = oss.str();

        item.first->toJson(js_children[prefix + "if"]);
        item.second->toJson(js_children[prefix + "then"]);
        ++prefix_num;
    }

    js["index"] = index_;
}

int IfThenAction::addChildAs(Action *child, const std::string &role) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return -1;
    }

    if (role == "if") {
        if (tmp_.first != nullptr) {
            LogWarn("%d:%s[%s], add child %d:%s[%s] as %s fail, need then",
                    id(), type().c_str(), label().c_str(),
                    child->id(), child->type().c_str(), child->label().c_str(),
                    role.c_str());
            return -1;
        }

        if (!child->setParent(this))
            return -1;

        child->setFinishCallback(std::bind(&IfThenAction::onIfActionFinished, this, _1));
        child->setBlockCallback(std::bind(&IfThenAction::block, this, _1, _2));
        tmp_.first = child;

        //! 先不处理，等到了then之后再加到if_then_actions_
        return if_then_actions_.size();

    } else if (role == "then") {
        if (tmp_.first == nullptr) {
            LogWarn("%d:%s[%s], add child %d:%s[%s] as %s fail, add if first",
                    id(), type().c_str(), label().c_str(),
                    child->id(), child->type().c_str(), child->label().c_str(),
                    role.c_str());
            return -1;
        }

        if (!child->setParent(this))
            return -1;

        child->setFinishCallback(std::bind(&IfThenAction::onLastChildFinished, this, _1, _2, _3));
        child->setBlockCallback(std::bind(&IfThenAction::block, this, _1, _2));
        tmp_.second = child;

        auto index = if_then_actions_.size();
        if_then_actions_.push_back(tmp_);

        tmp_.first = nullptr;
        tmp_.second = nullptr;

        return index;

    } else {
        LogWarn("%d:%s[%s], unsupport role:%s", id(), type().c_str(), label().c_str(), role.c_str());
        return -1;
    }
}

bool IfThenAction::isReady() const {
    return \
        tmp_.first == nullptr &&
        tmp_.second == nullptr &&
        !if_then_actions_.empty();
}

void IfThenAction::onStart() {
    SerialAssembleAction::onStart();

    index_ = 0;
    doStart();
}

void IfThenAction::onReset() {
    index_ = 0;

    for (auto &item : if_then_actions_) {
        item.first->reset();
        item.second->reset();
    }

    SerialAssembleAction::onReset();
}

void IfThenAction::doStart() {
    if (index_ >= if_then_actions_.size()) {
        finish(false, Reason(ACTION_REASON_IF_THEN_SKIP, "IfThenSkip"));
        return;
    }

    startThisAction(if_then_actions_.at(index_).first);
}

void IfThenAction::onIfActionFinished(bool is_succ) {
    if (handleChildFinishEvent([this, is_succ] { onIfActionFinished(is_succ); }))
        return;

    if (is_succ) {
        //! 执行then动作
        startThisAction(if_then_actions_.at(index_).second);

    } else {
        //! 跳至下一个分支
        ++index_;
        doStart();
    }
}

}
}
