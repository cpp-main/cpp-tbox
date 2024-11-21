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
#include "sequence_action.h"

#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

SequenceAction::SequenceAction(event::Loop &loop, Mode mode)
  : AssembleAction(loop, "Sequence")
  , mode_(mode)
{ }

SequenceAction::~SequenceAction() {
    for (auto action : children_)
        delete action;
}

void SequenceAction::toJson(Json &js) const {
    AssembleAction::toJson(js);
    Json &js_children = js["children"];
    for (auto action : children_) {
        Json js_child;
        action->toJson(js_child);
        js_children.push_back(std::move(js_child));
    }
    js["mode"] = ToString(mode_);
    js["index"] = index_;
}

int SequenceAction::addChild(Action *action) {
    TBOX_ASSERT(action != nullptr);

    if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
        int index = children_.size();
        children_.push_back(action);
        action->setFinishCallback(std::bind(&SequenceAction::onChildFinished, this, _1, _2, _3));
        action->setBlockCallback(std::bind(&SequenceAction::block, this, _1, _2));
        return index;
    } else {
        LogWarn("can't add child twice");
        return -1;
    }
}

bool SequenceAction::isReady() const {
    for (auto child : children_) {
        if (!child->isReady())
            return false;
    }
    return true;
}

void SequenceAction::onStart() {
    AssembleAction::onStart();

    startOtheriseFinish(true, Reason(), Trace());
}

void SequenceAction::onStop() {
    if (index_ < children_.size())
        children_.at(index_)->stop();

    AssembleAction::onStop();
}

void SequenceAction::onPause() {
    if (index_ < children_.size())
        children_.at(index_)->pause();

    AssembleAction::onPause();
}

void SequenceAction::onResume() {
    AssembleAction::onResume();

    if (index_ < children_.size())
        children_.at(index_)->resume();
}

void SequenceAction::onReset() {
    for (auto child : children_)
        child->reset();
    index_ = 0;

    AssembleAction::onReset();
}

void SequenceAction::startOtheriseFinish(bool is_succ, const Reason &reason, const Trace &trace) {
    if (index_ < children_.size()) {
        if (!children_.at(index_)->start())
            finish(false, Reason(ACTION_REASON_START_CHILD_FAIL, "StartChildFail"));
    } else {
        finish(is_succ, reason, trace);
    }
}

void SequenceAction::onChildFinished(bool is_succ, const Reason &reason, const Trace &trace) {
    if (state() == State::kRunning) {
        if ((mode_ == Mode::kAnySucc && is_succ) ||
            (mode_ == Mode::kAnyFail && !is_succ)) {
            finish(is_succ, reason, trace);
        } else {
            ++index_;
            startOtheriseFinish(is_succ, reason, trace);
        }
    }
}

std::string SequenceAction::ToString(Mode mode) {
    const char *tbl[] = { "AllFinish", "AnyFail", "AnySucc" };

    auto index = static_cast<size_t>(mode);
    if (index < NUMBER_OF_ARRAY(tbl))
        return tbl[index];

    return std::to_string(index);
}

}
}
