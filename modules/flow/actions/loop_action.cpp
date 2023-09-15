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
#include "loop_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

LoopAction::LoopAction(event::Loop &loop, Action *child, Mode mode) :
  Action(loop, "Loop"),
  child_(child),
  mode_(mode)
{
  TBOX_ASSERT(child != nullptr);
  child_->setFinishCallback(std::bind(&LoopAction::onChildFinished, this, _1));
}

LoopAction::~LoopAction() {
  delete child_;
}

void LoopAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["child"]);
}

bool LoopAction::onStart() {
  return child_->start();
}

bool LoopAction::onStop() {
  return child_->stop();
}

bool LoopAction::onPause() {
  return child_->pause();
}

bool LoopAction::onResume() {
  return child_->resume();
}

void LoopAction::onReset() {
  child_->reset();
}

void LoopAction::onChildFinished(bool is_succ) {
  if (state() == State::kRunning) {
    if ((mode_ == Mode::kUntilSucc && is_succ) ||
        (mode_ == Mode::kUntilFail && !is_succ)) {
      finish(is_succ);
    } else if (state() == State::kRunning) {
      child_->reset();
      child_->start();
    }
  }
}

}
}
