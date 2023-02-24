#include "repeat_action.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

RepeatAction::RepeatAction(event::Loop &loop, Action *child, size_t times, Mode mode) :
  Action(loop, "Repeat"),
  child_(child),
  repeat_times_(times),
  mode_(mode)
{
  TBOX_ASSERT(child != nullptr);

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

RepeatAction::~RepeatAction() {
  delete child_;
}

void RepeatAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["10.child"]);
  js["11.repeat_times"] = repeat_times_;
  js["12.remain_times"] = remain_times_;
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

}
}
