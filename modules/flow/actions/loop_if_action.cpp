#include "loop_if_action.h"

#include <base/json.hpp>
#include <base/assert.h>

namespace tbox {
namespace flow {

LoopIfAction::LoopIfAction(event::Loop &loop, Action *if_action, Action *exec_action) :
  Action(loop, "LoopIf"),
  if_action_(if_action),
  exec_action_(exec_action)
{
  TBOX_ASSERT(if_action != nullptr);
  TBOX_ASSERT(exec_action != nullptr);

  if_action_->setFinishCallback(
    [this] (bool is_succ) {
      if (is_succ) {
        exec_action_->start();
      } else {
        finish(finish_result_);
      }
    }
  );

  exec_action_->setFinishCallback(
    [this] (bool) {
      if (state() == State::kRunning) {
        if_action_->reset();
        exec_action_->reset();

        if (!if_action_->start())
          finish(finish_result_);
      }
    }
  );
}

LoopIfAction::~LoopIfAction() {
  delete if_action_;
  delete exec_action_;
}

void LoopIfAction::toJson(Json &js) const {
  Action::toJson(js);
  if_action_->toJson(js["0.if"]);
  exec_action_->toJson(js["1.exec"]);
}

bool LoopIfAction::onStart() {
  return if_action_->start();
}

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
