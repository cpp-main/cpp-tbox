#include "wrapper_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

WrapperAction::WrapperAction(event::Loop &loop)
  : Action(loop, "Wrapper")
{ }

void WrapperAction::setChild(Action::SharedPtr child) {
  TBOX_ASSERT(child != nullptr);

  child_ = child;
  child_->setFinishCallback([this](bool is_succ) { onChildFinished(is_succ); });
}

void WrapperAction::setMode(Mode mode) { mode_ = mode; }

void WrapperAction::toJson(Json &js) const {
  Action::toJson(js);
  js["mode"] = ToString(mode_);
  child_->toJson(js["child"]);
}

bool WrapperAction::onInit() { return child_ != nullptr; }

bool WrapperAction::onStart() { return child_->start(); }

bool WrapperAction::onStop() { return child_->stop(); }

bool WrapperAction::onPause() { return child_->pause(); }

bool WrapperAction::onResume() { return child_->resume(); }

void WrapperAction::onReset() { child_->reset(); }

void WrapperAction::onChildFinished(bool is_succ) {
  switch (mode_) {
    case Mode::kNormal: finish(is_succ); break;
    case Mode::kInvert: finish(!is_succ); break;
    case Mode::kAlwaySucc: finish(true); break;
    case Mode::kAlwayFail: finish(false); break;
    default: LogWarn("unsupport mode: %d", mode_);
  }
}

std::string ToString(WrapperAction::Mode mode) {
  const char *tbl[] = {"Normal", "Invert", "AlwaySucc", "AlwayFail"};
  auto idx = static_cast<size_t>(mode);
  if (idx >= 0 && idx < NUMBER_OF_ARRAY(tbl))
    return tbl[idx];
  else
    return "Unknown";
}

}
}
