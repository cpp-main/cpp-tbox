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
        kEpoll,
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
    virtual void runInLoop(const Func &func) = 0;
    virtual void runNext(const Func &func) = 0;
    /**
     * runInLoop() 与 runNext() 区别
     *
     * runInLoop()
     *   功能：注入下一轮将执行的函数，有加锁操作，支持跨线程，跨Loop间调用；
     *   场景：常用于不同Loop之间委派任务或其它线程向Loop线程妥派任务。
     *
     * runNext()
     *   功能：注入本回调完成后立即执行的函数，无加锁操作，不支持跨线程与跨Loop间调用；
     *   场景：常用于不方便在本函数中执行的操作，比如释放对象自身。
     *
     * runInLoop() 能替代 runNext()，但 runNext() 比 runInLoop() 更轻量。
     *
     * 建议：
     *   能用 runNext() 的场景就用 runNext()，不能用再使用 runInLoop()。
     *   如果你嫌麻烦，也不追求高效，一率采用 runInLoop() 也不会出错。
     */

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
