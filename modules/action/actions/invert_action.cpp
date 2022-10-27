#include "invert_action.h"

#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

InvertAction::InvertAction(event::Loop &loop, const std::string &id, Action *child) :
  Action(loop, id), child_(child)
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

bool InvertAction::onStart() {
  return child_->start();
}

bool InvertAction::onPause() {
  return child_->pause();
}

bool InvertAction::onResume() {
  return child_->resume();
}

bool InvertAction::onStop() {
  return child_->stop();
}

}
}
