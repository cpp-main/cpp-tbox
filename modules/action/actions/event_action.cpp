#include "event_action.h"

namespace tbox {
namespace action {

EventAction::EventAction(event::Loop &loop, EventPublisher &pub) :
  Action(loop), pub_(pub)
{ }

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
