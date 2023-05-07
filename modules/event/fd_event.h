#ifndef TBOX_EVENT_FD_ITEM_H_20170627
#define TBOX_EVENT_FD_ITEM_H_20170627

#include <functional>

#include "event.h"

namespace tbox {
namespace event {

class FdEvent : public Event {
  public:
    enum EventTypes {
        kReadEvent   = 0x01,
        kWriteEvent  = 0x02,
        kExceptEvent = 0x04,
    };

    using Event::Event;

    virtual bool initialize(int fd, short events, Mode mode) = 0;

    using CallbackFunc = std::function<void (short events)>;
    virtual void setCallback(CallbackFunc &&cb) = 0;

  public:
    virtual ~FdEvent() { }
};

}
}

#endif //TBOX_EVENT_FD_ITEM_H_20170627
