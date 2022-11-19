#include "sleep_action.h"

#include <tbox/base/assert.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace action {

SleepAction::SleepAction(event::Loop &loop, const std::chrono::milliseconds &time_span) :
  Action(loop),
  timer_(loop.newTimerEvent())
{
  assert(timer_ != nullptr);

  timer_->initialize(time_span, event::Event::Mode::kOneshot);
  timer_->setCallback([this] { finish(true); });
}

SleepAction::~SleepAction() {
  delete timer_;
}

bool SleepAction::onStart() {
  return timer_->enable();
}

bool SleepAction::onStop() {
  return timer_->disable();
}

bool SleepAction::onPause() {
  //FIXME:
  return timer_->disable();
}

bool SleepAction::onResume() {
  //FIXME:
  return timer_->enable();
}

void SleepAction::onReset() {
  timer_->disable();
}

}
}
