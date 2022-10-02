#include "sleep_for_action.h"

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace action {

SleepForAction::SleepForAction(Context &ctx, const std::chrono::milliseconds &time_span, bool is_done) :
  Action(ctx),
  timer_(ctx.loop().newTimerEvent()),
  is_done_(is_done)
{
  timer_->initialize(time_span, event::Event::Mode::kOneshot);
  timer_->setCallback([this] { finish(is_done_); });
}

SleepForAction::~SleepForAction() {
  delete timer_;
}

void SleepForAction::toJson(Json &js) const {
  Action::toJson(js);
}

bool SleepForAction::start() {
  if (!Action::start())
    return false;
  timer_->enable();
  return true;
}

bool SleepForAction::stop() {
  if (!Action::stop())
    return false;
  timer_->disable();
  return true;
}

}
}
