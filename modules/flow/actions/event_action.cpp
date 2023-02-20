#include "event_action.h"

namespace tbox {
namespace flow {

EventAction::EventAction(event::Loop &loop, const std::string &type, EventPublisher &pub) :
  Action(loop, type),
  pub_(pub)
{ }

EventAction::~EventAction() {
  if (state() == State::kRunning)
    pub_.unsubscribe(this);
}

bool EventAction::onStart() {
  pub_.subscribe(this);
  return true;
}

bool EventAction::onStop() {
  pub_.unsubscribe(this);
  return true;
}

bool EventAction::onPause() {
  pub_.unsubscribe(this);
  return true;
}

bool EventAction::onResume() {
  pub_.subscribe(this);
  return true;
}

void EventAction::onReset() {
  pub_.unsubscribe(this);
}

void EventAction::onFinished(bool succ) {
  pub_.unsubscribe(this);
}

}
}
