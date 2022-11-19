#include "dynamic_sleep_action.h"

#include <tbox/base/assert.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace action {

DynamicSleepAction::DynamicSleepAction(event::Loop &loop, const Generator &gen) :
  Action(loop),
  timer_(loop.newTimerEvent()),
  gen_(gen)
{
  assert(gen_ != nullptr);
  assert(timer_ != nullptr);

  timer_->setCallback([this] { finish(true); });
}

DynamicSleepAction::~DynamicSleepAction() {
  delete timer_;
}

bool DynamicSleepAction::onStart() {
  timer_->initialize(gen_(), event::Event::Mode::kOneshot);
  return timer_->enable();
}

bool DynamicSleepAction::onStop() {
  return timer_->disable();
}

bool DynamicSleepAction::onPause() {
  //FIXME:
  return timer_->disable();
}

bool DynamicSleepAction::onResume() {
  timer_->initialize(gen_(), event::Event::Mode::kOneshot);
  //FIXME:
  return timer_->enable();
}

void DynamicSleepAction::onReset() {
  timer_->disable();
}

}
}
