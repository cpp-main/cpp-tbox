#include "sequence_action.h"

#include <algorithm>
#include <tbox/base/log.h>

namespace tbox {
namespace action {

using namespace std::placeholders;

SequenceAction::SequenceAction(Context &ctx) :
  Action(ctx)
{ }

SequenceAction::~SequenceAction() {
  for (auto action : children_)
    delete action;
}

bool SequenceAction::append(Action *action) {
  if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
    children_.push_back(action);
    action->setFinishCallback(std::bind(&SequenceAction::onChildFinished, this, _1));
    return true;
  } else {
    LogWarn("can't add child twice");
    return false;
  }
}

bool SequenceAction::start() {
  if (!Action::start())
    return false;

  startOtheriseFinish();
  return true;
}

bool SequenceAction::stop() {
  if (!Action::stop())
    return false;

  if (index_ < children_.size())
    children_.at(index_)->stop();

  return true;
}

void SequenceAction::startOtheriseFinish() {
  if (index_ < children_.size()) {
    children_.at(index_)->start();
  } else {
    finish(true);
  }
}

void SequenceAction::onChildFinished(bool is_done) {
  if (is_done) {
    ++index_;
    startOtheriseFinish();
  } else {
    finish(false);
  }
}

}
}
