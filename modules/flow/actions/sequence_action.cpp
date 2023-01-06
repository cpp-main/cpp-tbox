#include "sequence_action.h"

#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace flow {

using namespace std::placeholders;

SequenceAction::~SequenceAction() {
  for (auto action : children_)
    delete action;
}

void SequenceAction::toJson(Json &js) const {
  Action::toJson(js);
  Json &js_children = js["children"];
  for (auto action : children_) {
    Json js_child;
    action->toJson(js_child);
    js_children.push_back(std::move(js_child));
  }
}

int SequenceAction::append(Action *action) {
  TBOX_ASSERT(action != nullptr);

  if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
    int index = children_.size();
    children_.push_back(action);
    action->setFinishCallback(std::bind(&SequenceAction::onChildFinished, this, _1));
    return index;
  } else {
    LogWarn("can't add child twice");
    return -1;
  }
}

bool SequenceAction::onStart() {
  startOtheriseFinish();
  return true;
}

bool SequenceAction::onStop() {
  if (index_ < children_.size())
    return children_.at(index_)->stop();
  return false;
}

bool SequenceAction::onPause() {
  if (index_ < children_.size())
    return children_.at(index_)->pause();
  return false;
}

bool SequenceAction::onResume() {
  if (index_ < children_.size())
    return children_.at(index_)->resume();
  return false;
}

void SequenceAction::onReset() {
  for (auto child : children_)
    child->reset();

  index_ = 0;
}

void SequenceAction::startOtheriseFinish() {
  if (index_ < children_.size()) {
    if (!children_.at(index_)->start())
      finish(false);
  } else {
    finish(true);
  }
}

void SequenceAction::onChildFinished(bool is_succ) {
  if ((finish_condition_ == FinishCondition::kAnySucc && is_succ) ||
      (finish_condition_ == FinishCondition::kAnyFail && !is_succ)) {
    finish(is_succ);
  } else {
    ++index_;
    startOtheriseFinish();
  }
}

}
}
