#include "weekly_timer.h"

namespace tbox {
namespace timer {

WeeklyTimer::WeeklyTimer(event::Loop *wp_loop) { }

WeeklyTimer::~WeeklyTimer() { }

bool WeeklyTimer::initialize(const std::string &week_mask, uint32_t seconds_of_day, int timezone_offset_minutes) {
  return false;
}

bool WeeklyTimer::isEnabled() const {
  return false;
}

bool WeeklyTimer::enable() {
  return false;
}

bool WeeklyTimer::disable() {
  return false;
}

void WeeklyTimer::cleanup() { }

}
}
