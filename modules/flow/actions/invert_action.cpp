#include "invert_action.h"

#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

InvertAction::InvertAction(event::Loop &loop, Action *child) :
  Action(loop), child_(child)
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

bool InvertAction::onStop() {
  return child_->stop();
}

bool InvertAction::onPause() {
  return child_->pause();
}

bool InvertAction::onResume() {
  return child_->resume();
}

void InvertAction::onReset() {
  child_->reset();
}

}
}
