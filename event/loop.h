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

    using Func = std::function<void()>;
    //! 注入下一轮将执行的函数，支持跨线程
    virtual void runInLoop(const Func &func) = 0;
    //! 注入本回调完成后立即执行的函数，不支持跨线程
    virtual void runNext(const Func &func) = 0;

    virtual FdEvent* newFdEvent() = 0;
    virtual TimerEvent* newTimerEvent() = 0;
    virtual SignalEvent* newSignalEvent() = 0;

    virtual Stat getStat() const = 0;
    virtual void resetStat() = 0;

  public:
    virtual ~Loop() { }
};

}
}

#endif //TBOX_EVENT_LOOP_H
