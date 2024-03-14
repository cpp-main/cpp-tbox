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

ParallelAction::ParallelAction(event::Loop &loop)
  : AssembleAction(loop, "Parallel")
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
}

int ParallelAction::addChild(Action *action) {
    TBOX_ASSERT(action != nullptr);

    if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
        int index = children_.size();
        children_.push_back(action);
        action->setFinishCallback(std::bind(&ParallelAction::onChildFinished, this, index, _1));
        action->setBlockCallback(std::bind(&ParallelAction::onChildBlocked, this, index, _1));
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
        if (!action->start())
            finished_children_[index] = false;
    }

    tryFinish();
}

void ParallelAction::onStop() {
    for (Action *action : children_)
        action->stop();

    AssembleAction::onStop();
}

void ParallelAction::onPause() {
    for (Action *action : children_) {
        if (action->state() == State::kRunning)
            action->pause();
    }

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
    blocked_children_.clear();

    AssembleAction::onReset();
}

void ParallelAction::tryFinish() {
    if ((finished_children_.size() + blocked_children_.size()) == children_.size())
        finish(true);
}

void ParallelAction::onChildFinished(int index, bool is_succ) {
    if (state() == State::kRunning) {
        finished_children_[index] = is_succ;
        tryFinish();
    }
}

void ParallelAction::onChildBlocked(int index, const Reason &why) {
    if (state() == State::kRunning) {
        blocked_children_[index] = why;
        //!FIXME:目前遇到block的动作，仅停止它。将来需要更精细化的处理
        children_.at(index)->stop();
        tryFinish();
    }
}

}
}
