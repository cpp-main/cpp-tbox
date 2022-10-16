#include "invert_action.h"

#include <cassert>
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

InvertAction::InvertAction(Context &ctx, const std::string &name, Action *child) :
  Action(ctx, name), child_(child)
{
  assert(child_ != nullptr);

  child_->setFinishCallback(
    [this] (bool is_succ){
      finish(!is_succ);
    }
  );
}

InvertAction::~InvertAction() {
  delete child_;
}

void InvertAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["child"]);
}

bool InvertAction::start() {
  return Action::start() && child_->start();
}

bool InvertAction::pause() {
  return Action::pause() && child_->pause();
}

bool InvertAction::resume() {
  return Action::resume() && child_->resume();
}

bool InvertAction::stop() {
  return Action::stop() && child_->stop();
}

}
}
