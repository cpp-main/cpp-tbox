#include "loop_if_action.h"

#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

LoopIfAction::LoopIfAction(event::Loop &loop)
  : Action(loop, "LoopIf")
{}

void LoopIfAction::setIfAction(Action::SharedPtr action)
{
  if_action_ = action;
  if (if_action_) {
    if_action_->setFinishCallback(
      [this] (bool is_succ) {
        if (is_succ) {
          exec_action_->start();
        } else {
          finish(true);
        }
      }
    );
  }
}

void LoopIfAction::setExecAction(Action::SharedPtr action)
{
  exec_action_ = action;
  if (exec_action_) {
    exec_action_->setFinishCallback(
      [this] (bool) {
        if (state() == State::kRunning) {
          if_action_->reset();
          exec_action_->reset();

          if (!if_action_->start())
            finish(true);
        }
      }
    );
  }
}

void LoopIfAction::toJson(Json &js) const {
  Action::toJson(js);
  if_action_->toJson(js["0.if"]);
  exec_action_->toJson(js["1.exec"]);
}

bool LoopIfAction::onInit() { return if_action_ && exec_action_; }

bool LoopIfAction::onStart() { return if_action_->start(); }

bool LoopIfAction::onStop() {
  auto curr_action = if_action_->state() == State::kFinished ? exec_action_ : if_action_;
  return curr_action->stop();
}

bool LoopIfAction::onPause() {
  auto curr_action = if_action_->state() == State::kFinished ? exec_action_ : if_action_;
  return curr_action->pause();
}

bool LoopIfAction::onResume() {
  auto curr_action = if_action_->state() == State::kFinished ? exec_action_ : if_action_;
  return curr_action->resume();
}

void LoopIfAction::onReset() {
  if_action_->reset();
  exec_action_->reset();
}

}
}
