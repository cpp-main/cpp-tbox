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
#include "parallel_action.h"

#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

ParallelAction::ParallelAction(event::Loop &loop, Mode mode)
  : AssembleAction(loop, "Parallel")
  , mode_(mode)
{ }

ParallelAction::~ParallelAction() {
    for (auto action : children_)
        delete action;
}

void ParallelAction::toJson(Json &js) const {
    AssembleAction::toJson(js);
    Json &js_children = js["children"];
    for (auto action : children_) {
        Json js_child;
        action->toJson(js_child);
        js_children.push_back(std::move(js_child));
    }
    js["mode"] = ToString(mode_);
}

int ParallelAction::addChild(Action *action) {
    if (action == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return -1;
    }

    if (action->setParent(this)) {
        int index = children_.size();
        action->setFinishCallback(std::bind(&ParallelAction::onChildFinished, this, index, _1));
        action->setBlockCallback(std::bind(&ParallelAction::onChildBlocked, this, index, _1, _2));
        children_.push_back(action);
        return index;

    } else {
        LogWarn("can't add child twice");
        return -1;
    }
}

bool ParallelAction::isReady() const {
    for (auto child : children_) {
        if (!child->isReady())
            return false;
    }
    return true;
}

void ParallelAction::onStart() {
    AssembleAction::onStart();

    for (size_t index = 0; index < children_.size(); ++index) {
        Action *action = children_.at(index);
        if (!action->start()) {
            finished_children_[index] = false;
            //! 如果是任一失败都退出，那么要直接结束
            if (mode_ == Mode::kAnyFail) {
                finish(true);
                stopAllActions();
                return;
            }
        }
    }

    //! 如果全部都启动失败了，那么就直接结束
    if (finished_children_.size() == children_.size())
        finish(true);
}

void ParallelAction::onStop() {
    stopAllActions();
    AssembleAction::onStop();
}

void ParallelAction::onPause() {
    pauseAllActions();
    AssembleAction::onPause();
}

void ParallelAction::onResume() {
    AssembleAction::onResume();

    for (Action *action : children_) {
        if (action->state() == State::kPause)
            action->resume();
    }
}

void ParallelAction::onReset() {
    for (auto child : children_)
        child->reset();

    finished_children_.clear();
    AssembleAction::onReset();
}

void ParallelAction::stopAllActions() {
    for (Action *action : children_) {
        action->stop();
    }
}

void ParallelAction::pauseAllActions() {
    for (Action *action : children_) {
        action->pause();
    }
}

void ParallelAction::onChildFinished(int index, bool is_succ) {
    if (state() == State::kRunning) {
        finished_children_[index] = is_succ;

        if ((mode_ == Mode::kAnySucc && is_succ) ||
            (mode_ == Mode::kAnyFail && !is_succ)) {
            stopAllActions();
            finish(true);

        } else if (finished_children_.size() == children_.size()) {
            finish(true);
        }
    }
}

void ParallelAction::onChildBlocked(int, const Reason &why, const Trace &trace) {
    if (state() == State::kRunning) {
        pauseAllActions();
        block(why, trace);
    }
}

std::string ParallelAction::ToString(Mode mode) {
    const char *tbl[] = { "AllFinish", "AnyFail", "AnySucc" };

    auto index = static_cast<size_t>(mode);
    if (index < NUMBER_OF_ARRAY(tbl))
        return tbl[index];

    return std::to_string(index);
}

}
}
