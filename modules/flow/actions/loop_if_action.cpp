#include "loop_if_action.h"

#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

LoopIfAction::LoopIfAction(event::Loop &loop, Action *cond_action, Action *exec_action) :
  Action(loop),
  cond_action_(cond_action),
  exec_action_(exec_action)
{
  assert(cond_action != nullptr);
  assert(exec_action != nullptr);

  cond_action_->setFinishCallback(
    [this] (bool is_succ) {
      if (is_succ) {
        exec_action_->start();
        is_cond_done_ = true;
      } else {
        finish(finish_result_);
      }
    }
  );

  exec_action_->setFinishCallback(
    [this] (bool) {
      if (state() == State::kRunning) {
        cond_action_->reset();
        exec_action_->reset();
        is_cond_done_ = false;

        if (!cond_action_->start())
          finish(finish_result_);
      }
    }
  );
}

LoopIfAction::~LoopIfAction() {
  delete cond_action_;
  delete exec_action_;
}

void LoopIfAction::toJson(Json &js) const {
  Action::toJson(js);
  cond_action_->toJson(js["cond"]);
  exec_action_->toJson(js["exec"]);
  js["is_cond_done"] = is_cond_done_;
}

bool LoopIfAction::onStart() {
  return cond_action_->start();
}

bool LoopIfAction::onStop() {
  auto curr_action = is_cond_done_ ? exec_action_ : cond_action_;
  return curr_action->stop();
}

bool LoopIfAction::onPause() {
  auto curr_action = is_cond_done_ ? exec_action_ : cond_action_;
  return curr_action->pause();
}

bool LoopIfAction::onResume() {
  auto curr_action = is_cond_done_ ? exec_action_ : cond_action_;
  return curr_action->resume();
}

void LoopIfAction::onReset() {
  cond_action_->reset();
  exec_action_->reset();
  is_cond_done_ = false;
}

}
}
