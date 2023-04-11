#ifndef TBOX_EVENT_SIGNAL_ITEM_H_20170627
#define TBOX_EVENT_SIGNAL_ITEM_H_20170627

#include <signal.h>
#include <functional>
#include <set>

#include "event.h"

namespace tbox {
namespace event {

class SignalEvent : public Event {
  public:
    using Event::Event;

    virtual bool initialize(int signum, Mode mode) = 0;
    virtual bool initialize(const std::set<int> &sigset, Mode mode) = 0;
    virtual bool initialize(const std::initializer_list<int> &sigset, Mode mode) = 0;

    using CallbackFunc = std::function<void (int)>;
    virtual void setCallback(const CallbackFunc &cb) = 0;

  public:
    virtual ~SignalEvent() { }
};

}
}

#endif //TBOX_EVENT_SIGNAL_ITEM_H_20170627
