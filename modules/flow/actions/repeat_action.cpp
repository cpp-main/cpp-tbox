#include "repeat_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

RepeatAction::RepeatAction(event::Loop &loop)
  : Action(loop, "Repeat")
{ }

void RepeatAction::setChild(Action::SharedPtr child)
{
  child_ = child;
  if (child_) {
    child_->setFinishCallback(
      [this] (bool is_succ) {
        if ((mode_ == Mode::kBreakSucc && is_succ) ||
            (mode_ == Mode::kBreakFail && !is_succ)) {
          finish(is_succ);
        } else if (state() == State::kRunning) {
          if (remain_times_ > 0) {
            child_->reset();
            child_->start();
            --remain_times_;
          } else {
            finish(true);
          }
        }
      }
    );
  }
}

void RepeatAction::setMode(Mode mode) { mode_ = mode; }

void RepeatAction::setRepeatTimes(size_t repeat_times) { repeat_times_ = repeat_times; }

void RepeatAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["child"]);
  js["repeat_times"] = repeat_times_;
  js["remain_times"] = remain_times_;
}

bool RepeatAction::onInit() { return child_ != nullptr; }

bool RepeatAction::onStart() {
  remain_times_ = repeat_times_ - 1;
  return child_->start();
}

bool RepeatAction::onStop() { return child_->stop(); }

bool RepeatAction::onPause() { return child_->pause(); }

bool RepeatAction::onResume() { return child_->resume(); }

void RepeatAction::onReset() { child_->reset(); }

}
}
