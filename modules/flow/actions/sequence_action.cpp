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

SequenceAction::SequenceAction(event::Loop &loop) :
  Action(loop, "Sequence")
{ }

SequenceAction::~SequenceAction() {
  for (auto action : children_)
    delete action;
}

void SequenceAction::toJson(Json &js) const {
  Action::toJson(js);
  Json &js_children = js["children"];
  for (auto action : children_) {
    Json js_child;
    action->toJson(js_child);
    js_children.push_back(std::move(js_child));
  }
  js["index"] = index_;
}

int SequenceAction::append(Action *action) {
  TBOX_ASSERT(action != nullptr);

  if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
    int index = children_.size();
    children_.push_back(action);
    action->setFinishCallback(std::bind(&SequenceAction::onChildFinished, this, _1));
    return index;
  } else {
    LogWarn("can't add child twice");
    return -1;
  }
}

bool SequenceAction::onStart() {
  startOtheriseFinish(true);
  return true;
}

bool SequenceAction::onStop() {
  if (index_ < children_.size())
    return children_.at(index_)->stop();
  return false;
}

bool SequenceAction::onPause() {
  if (index_ < children_.size())
    return children_.at(index_)->pause();
  return false;
}

bool SequenceAction::onResume() {
  if (index_ < children_.size())
    return children_.at(index_)->resume();
  return false;
}

void SequenceAction::onReset() {
  for (auto child : children_)
    child->reset();

  index_ = 0;
}

void SequenceAction::startOtheriseFinish(bool is_succ) {
  if (index_ < children_.size()) {
    if (!children_.at(index_)->start())
      finish(false);
  } else {
    finish(is_succ);
  }
}

void SequenceAction::onChildFinished(bool is_succ) {
  if (state() == State::kRunning) {
    if ((finish_condition_ == FinishCondition::kAnySucc && is_succ) ||
        (finish_condition_ == FinishCondition::kAnyFail && !is_succ)) {
      finish(is_succ);
    } else {
      ++index_;
      startOtheriseFinish(is_succ);
    }
  }
}

}
}
