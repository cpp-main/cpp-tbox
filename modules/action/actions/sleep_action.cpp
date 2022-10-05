#include "sleep_action.h"

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace action {

SleepAction::SleepAction(Context &ctx, const std::chrono::milliseconds &time_span) :
  Action(ctx),
  timer_(ctx.loop().newTimerEvent())
{
  timer_->initialize(time_span, event::Event::Mode::kOneshot);
  timer_->setCallback([this] { finish(true); });
}

SleepAction::~SleepAction() {
  delete timer_;
}

bool SleepAction::start() {
  if (!Action::start())
    return false;
  timer_->enable();
  return true;
}

bool SleepAction::stop() {
  if (!Action::stop())
    return false;
  timer_->disable();
  return true;
}

}
}
