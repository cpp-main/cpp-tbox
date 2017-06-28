#ifndef TBOX_EVENT_FD_ITEM_H_20170627
#define TBOX_EVENT_FD_ITEM_H_20170627

#include <functional>

#include "item.h"

namespace tbox {
namespace event {

class FdItem : public Item {
  public:
    static const int kReadEvent   = 0x01;
    static const int kWriteEvent  = 0x02;
    static const int kExceptEvent = 0x04;

    virtual bool initialize(int fd, short events, Mode mode) = 0;

    using CallbackFunc = std::function<void (short events)>;
    virtual void setCallback(const CallbackFunc &cb) = 0;

  public:
    virtual ~FdItem() { }
};

}
}

#endif //TBOX_EVENT_FD_ITEM_H_20170627
