#ifndef TBOX_EVENT_LOOP_H_20170627
#define TBOX_EVENT_LOOP_H_20170627

#include <functional>
#include "forward.h"

namespace tbox {
namespace event {

class Loop {
  public:
    enum class Engine {
        kLibevent,
        kLibev,
        kLibuv,
    };
    static Loop* New(Engine type);

    enum class Mode {
        kOnce,
        kForever
    };

    virtual void runLoop(Mode mode = Mode::kForever) = 0;

    virtual void exitLoop() = 0;

    virtual FdItem* newFdItem() = 0;
    virtual TimerItem* newTimerItem() = 0;
    virtual SignalItem* newSignalItem() = 0;

  public:
    virtual ~Loop() { }
};

}
}

#endif //TBOX_EVENT_LOOP_H_20170627
