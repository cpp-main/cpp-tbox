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
#include "repeat_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

RepeatAction::RepeatAction(event::Loop &loop, Action *child, size_t times, Mode mode) :
  Action(loop, "Repeat"),
  child_(child),
  repeat_times_(times),
  mode_(mode)
{
  TBOX_ASSERT(child != nullptr);
  child_->setFinishCallback(std::bind(&RepeatAction::onChildFinished, this, _1));
}

RepeatAction::~RepeatAction() {
  delete child_;
}

void RepeatAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["child"]);
  js["repeat_times"] = repeat_times_;
  js["remain_times"] = remain_times_;
}

bool RepeatAction::onStart() {
  remain_times_ = repeat_times_ - 1;
  return child_->start();
}

bool RepeatAction::onStop() {
  return child_->stop();
}

bool RepeatAction::onPause() {
  return child_->pause();
}

bool RepeatAction::onResume() {
  return child_->resume();
}

void RepeatAction::onReset() {
  child_->reset();
}

void RepeatAction::onChildFinished(bool is_succ) {
  if (state() == State::kRunning) {
    if ((mode_ == Mode::kBreakSucc && is_succ) ||
        (mode_ == Mode::kBreakFail && !is_succ)) {
      finish(is_succ);
    } else {
      if (remain_times_ > 0) {
        child_->reset();
        child_->start();
        --remain_times_;
      } else {
        finish(true);
      }
    }
  }
}

}
}
