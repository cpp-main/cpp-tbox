#include "loop_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

LoopAction::LoopAction(event::Loop &loop, Action *child, Mode mode) :
  Action(loop, "Loop"),
  child_(child),
  mode_(mode)
{
  TBOX_ASSERT(child != nullptr);

  child_->setFinishCallback(
    [this] (bool is_succ) {
      if ((mode_ == Mode::kUntilSucc && is_succ) ||
          (mode_ == Mode::kUntilFail && !is_succ)) {
        finish(true);
      } else if (state() == State::kRunning) {
        child_->reset();
        child_->start();
      }
    }
  );
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

}
}
