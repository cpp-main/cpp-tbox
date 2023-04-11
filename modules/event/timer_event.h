#ifndef TBOX_EVENT_TIMER_ITEM_H_20170627
#define TBOX_EVENT_TIMER_ITEM_H_20170627

#include <functional>
#include <chrono>

#include "event.h"

namespace tbox {
namespace event {

class TimerEvent : public Event {
  public:
    using Event::Event;

    virtual bool initialize(const std::chrono::milliseconds &time_span, Mode mode) = 0;

    using CallbackFunc = std::function<void ()>;
    virtual void setCallback(const CallbackFunc &cb) = 0;

  public:
    virtual ~TimerEvent() { }
};

}
}

#endif //TBOX_EVENT_TIMER_ITEM_H_20170627
