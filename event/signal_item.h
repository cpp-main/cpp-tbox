#ifndef TBOX_EVENT_SIGNAL_ITEM_H_20170627
#define TBOX_EVENT_SIGNAL_ITEM_H_20170627

#include <signal.h>
#include <functional>

#include "item.h"

namespace tbox {
namespace event {

class SignalItem : public Item {
  public:
    virtual bool initialize(int signum, Mode mode) = 0;

    using CallbackFunc = std::function<void ()>;
    virtual void setCallback(const CallbackFunc &cb) = 0;

  public:
    virtual ~SignalItem() { }
};

}
}

#endif //TBOX_EVENT_SIGNAL_ITEM_H_20170627
