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

IfElseAction::IfElseAction(event::Loop &loop, Action *if_action,
                           Action *succ_action, Action *fail_action) :
  Action(loop, "IfElse"),
  if_action_(if_action),
  succ_action_(succ_action),
  fail_action_(fail_action)
{
  TBOX_ASSERT(if_action != nullptr);

  if_action_->setFinishCallback(std::bind(&IfElseAction::onCondActionFinished, this, _1));
  if (succ_action_ != nullptr)
    succ_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
  if (fail_action_ != nullptr)
    fail_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
}

IfElseAction::~IfElseAction() {
  CHECK_DELETE_RESET_OBJ(if_action_);
  CHECK_DELETE_RESET_OBJ(succ_action_);
  CHECK_DELETE_RESET_OBJ(fail_action_);
}

void IfElseAction::toJson(Json &js) const {
  Action::toJson(js);
  auto &js_children = js["children"];
  if_action_->toJson(js_children["0.if"]);
  if (succ_action_ != nullptr)
    succ_action_->toJson(js_children["1.succ"]);
  if (fail_action_ != nullptr)
    fail_action_->toJson(js_children["2.fail"]);
}

bool IfElseAction::onStart() {
  return if_action_->start();
}

bool IfElseAction::onStop() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess)
      return succ_action_->stop();
    else
      return fail_action_->stop();
  } else {
    return if_action_->stop();
  }
}

bool IfElseAction::onPause() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess)
      return succ_action_->pause();
    else
      return fail_action_->pause();
  } else {
    return if_action_->pause();
  }
}

bool IfElseAction::onResume() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess) {
      if (succ_action_->state() == State::kFinished) {
        finish(succ_action_->result() == Result::kSuccess);
        return true;
      } else {
        return succ_action_->resume();
      }
    } else {
      if (fail_action_->state() == State::kFinished) {
        finish(fail_action_->result() == Result::kSuccess);
        return true;
      } else {
        return fail_action_->resume();
      }
    }
  } else {
    return if_action_->resume();
  }
}

void IfElseAction::onReset() {
  if_action_->reset();

  if (succ_action_ != nullptr)
    succ_action_->reset();

  if (fail_action_ != nullptr)
    fail_action_->reset();
}

void IfElseAction::onCondActionFinished(bool is_succ) {
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
    finish(true);
  }
}

}
}
