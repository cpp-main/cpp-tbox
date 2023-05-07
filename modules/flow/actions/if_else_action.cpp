#include "if_else_action.h"

#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

IfElseAction::IfElseAction(event::Loop &loop)
  : Action(loop, "IfElse")
{ }

void IfElseAction::setIfAction(Action::SharedPtr action) {
  TBOX_ASSERT(action != nullptr);

  if_action_ = action;
  if_action_->setFinishCallback(std::bind(&IfElseAction::onCondActionFinished, this, _1));
}

void IfElseAction::setSuccAction(Action::SharedPtr action) {
  succ_action_ = action;

  if (succ_action_)
    succ_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
}

void IfElseAction::setFailAction(Action::SharedPtr action) {
  fail_action_ = action;

  if (fail_action_)
    fail_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
}

void IfElseAction::toJson(Json &js) const {
  Action::toJson(js);

  auto &js_children = js["children"];
  if_action_->toJson(js_children["0.if"]);

  if (succ_action_)
    succ_action_->toJson(js_children["1.succ"]);

  if (fail_action_)
    fail_action_->toJson(js_children["2.fail"]);
}

bool IfElseAction::onInit() {
  return \
    (if_action_) && \
    (succ_action_ || fail_action_);
}

bool IfElseAction::onStart() {
  return if_action_->start();
}

bool IfElseAction::onStop() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess) {
      if (succ_action_)
        return succ_action_->stop();
    } else {
      if (fail_action_)
        return fail_action_->stop();
    }
  } else {
    return if_action_->stop();
  }

  return false;
}

bool IfElseAction::onPause() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess) {
      if (succ_action_)
        return succ_action_->pause();
    } else {
      if (fail_action_)
        return fail_action_->pause();
    }
  } else {
    return if_action_->pause();
  }

  return false;
}

bool IfElseAction::onResume() {
  if (if_action_->state() == Action::State::kFinished) {
    if (if_action_->result() == Action::Result::kSuccess)
      return succ_action_->resume();
    else
      return fail_action_->resume();
  } else {
    return if_action_->resume();
  }
}

void IfElseAction::onReset() {
  if_action_->reset();

  if (succ_action_)
    succ_action_->reset();

  if (fail_action_)
    fail_action_->reset();
}

void IfElseAction::onCondActionFinished(bool is_succ) {
  if (is_succ) {
    if (succ_action_) {
      succ_action_->start();
      return;
    }
  } else {
    if (fail_action_) {
      fail_action_->start();
      return;
    }
  }

  finish(true);
}

}
}
