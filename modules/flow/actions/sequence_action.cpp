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
  : SerialAssembleAction(loop, "Sequence")
  , mode_(mode)
{ }

SequenceAction::~SequenceAction() {
    for (auto action : children_)
        delete action;
}

void SequenceAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);

    Json &js_children = js["children"];
    for (auto action : children_) {
        Json js_child;
        action->toJson(js_child);
        js_children.push_back(std::move(js_child));
    }

    js["mode"] = ToString(mode_);
    js["index"] = index_;
}

int SequenceAction::addChild(Action *child) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    int index = children_.size();
    child->setFinishCallback(std::bind(&SequenceAction::onChildFinished, this, _1, _2, _3));
    child->setBlockCallback(std::bind(&SequenceAction::block, this, _1, _2));
    children_.push_back(child);

    return index;
}

bool SequenceAction::isReady() const {
    for (auto child : children_) {
        if (!child->isReady())
            return false;
    }
    return true;
}

void SequenceAction::onStart() {
    SerialAssembleAction::onStart();

    startOtheriseFinish(true, Reason(), Trace());
}

void SequenceAction::onReset() {
    index_ = 0;
    for (auto child : children_)
        child->reset();

    SerialAssembleAction::onReset();
}

void SequenceAction::startOtheriseFinish(bool is_succ, const Reason &reason, const Trace &trace) {
    if (index_ < children_.size()) {
        if (!startThisAction(children_.at(index_)))
            finish(false, Reason(ACTION_REASON_START_CHILD_FAIL, "StartChildFail"));

    } else {
        finish(is_succ, reason, trace);
    }
}

void SequenceAction::onChildFinished(bool is_succ, const Reason &reason, const Trace &trace) {
    if (handleChildFinishEvent([this, is_succ, reason, trace] { onChildFinished(is_succ, reason, trace); }))
        return;

    if ((mode_ == Mode::kAnySucc && is_succ) ||
        (mode_ == Mode::kAnyFail && !is_succ)) {
        finish(is_succ, reason, trace);

    } else {
        ++index_;
        startOtheriseFinish(is_succ, reason, trace);
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
