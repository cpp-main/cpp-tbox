#ifndef TBOX_ACTION_CONTEXT_H_20221002
#define TBOX_ACTION_CONTEXT_H_20221002

#include <tbox/event/loop.h>
#include "event_publisher.h"

namespace tbox {
namespace action {

class Context {
  public:
    virtual ~Context() { }

  public:
    virtual event::Loop& loop() = 0;  
    virtual EventPublisher& event_publisher() = 0;
};

}
}

#endif //TBOX_ACTION_CONTEXT_H_20221002
