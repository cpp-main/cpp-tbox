#include "fallback_action.h"

#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

using namespace std::placeholders;

FallbackAction::~FallbackAction() {
  for (auto action : children_)
    delete action;
}

void FallbackAction::toJson(Json &js) const {
  Action::toJson(js);
  Json &js_children = js["children"];
  for (auto action : children_) {
    Json js_child;
    action->toJson(js_child);
    js_children.push_back(std::move(js_child));
  }
}

int FallbackAction::append(Action *action) {
  assert(action != nullptr);

  if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
    int index = children_.size();
    children_.push_back(action);
    action->setFinishCallback(std::bind(&FallbackAction::onChildFinished, this, _1));
    return index;
  } else {
    LogWarn("can't add child twice");
    return -1;
  }
}

bool FallbackAction::onStart() {
  startOtheriseFinish();
  return true;
}

bool FallbackAction::onStop() {
  if (index_ < children_.size())
    children_.at(index_)->stop();
  return true;
}

bool FallbackAction::onStop() {
  if (index_ < children_.size())
    children_.at(index_)->stop();
  return true;
}

bool FallbackAction::onPause() {
  if (index_ < children_.size())
    children_.at(index_)->pause();
  return true;
}

bool FallbackAction::onResume() {
  if (index_ < children_.size())
    children_.at(index_)->resume();
  return true;
}

void FallbackAction::onReset() {
  for (auto child : children_)
    child->reset();

  index_ = 0;
}

void FallbackAction::startOtheriseFinish() {
  if (index_ < children_.size()) {
    children_.at(index_)->start();
  } else {
    finish(false);
  }
}

void FallbackAction::onChildFinished(bool is_succ) {
  if (is_succ) {
    finish(true);
  } else {
    ++index_;
    startOtheriseFinish();
  }
}

}
}
