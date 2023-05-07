#include "composite_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

void CompositeAction::setChild(Action::SharedPtr child) {
  TBOX_ASSERT(child_ != nullptr);

  child_ = child;
  child_->setFinishCallback( [this](bool succ) { finish(succ); });
}

void CompositeAction::toJson(Json &js) const {
  Action::toJson(js);
  child_->toJson(js["child"]);
}

bool CompositeAction::onInit() { return child_ != nullptr; }

bool CompositeAction::onStart() { return child_->start(); }

bool CompositeAction::onStop() { return child_->stop(); }

bool CompositeAction::onPause() { return child_->pause(); }

bool CompositeAction::onResume() { return child_->resume(); }

void CompositeAction::onReset() { child_->reset(); }

}
}
