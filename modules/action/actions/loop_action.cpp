#include "loop_action.h"
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

LoopAction::LoopAction(Context &ctx, const std::string &name, Action *child, Mode mode) :
  Action(ctx, name),
  child_(child),
  mode_(mode)
{
  child_->setFinishCallback(
    [this] (bool is_succ) {
      if ((mode_ == Mode::kUntilSucc && is_succ) ||
          (mode_ == Mode::kUntilFail && !is_succ)) {
        finish(true);
      } else {
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

bool LoopAction::start() {
  return Action::start() && child_->start();
}

bool LoopAction::pause() {
  return Action::pause() && child_->pause();
}

bool LoopAction::resume() {
  return Action::resume() && child_->resume();
}

bool LoopAction::stop() {
  return Action::stop() && child_->stop();
}

}
}
