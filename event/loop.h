#ifndef TBOX_EVENT_LOOP_H
#define TBOX_EVENT_LOOP_H

#include <functional>
#include <chrono>

#include "forward.h"
#include "stat.h"

namespace tbox {
namespace event {

class Loop {
  public:
    enum class Engine {
        kLibevent,
        kLibev,
        kLibuv,
    };

    static Loop* New();
    static Loop* New(Engine type);

    enum class Mode {
        kOnce,
        kForever
    };

    virtual void runLoop(Mode mode = Mode::kForever) = 0;

    virtual void exitLoop(const std::chrono::milliseconds &wait_time = std::chrono::milliseconds::zero()) = 0;

    virtual bool isInLoopThread() = 0;

    using RunInLoopFunc = std::function<void()>;
    virtual void runInLoop(const RunInLoopFunc &func) = 0;

    virtual FdItem* newFdItem() = 0;
    virtual TimerItem* newTimerItem() = 0;
    virtual SignalItem* newSignalItem() = 0;

    virtual Stat getStat() const = 0;
    virtual void resetStat() = 0;

  public:
    virtual ~Loop() { }
};

}
}

#endif //TBOX_EVENT_LOOP_H
