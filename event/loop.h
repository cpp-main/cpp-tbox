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

    //! 创建默认类型的事件循环
    static Loop* New();
    //! 创建指定类型的事件循环
    static Loop* New(Engine type);

    enum class Mode {
        kOnce,      //!< 仅执行一次
        kForever    //!< 一直执行
    };

    //! 执行事件循环
    virtual void runLoop(Mode mode = Mode::kForever) = 0;
    //! 退出事件循环
    virtual void exitLoop(const std::chrono::milliseconds &wait_time = std::chrono::milliseconds::zero()) = 0;

    //! 是否与Loop在同一个线程内
    virtual bool isInLoopThread() = 0;
    //! Loop是否正在运行
    virtual bool isRunning() const = 0;

    //! 委托延后执行动作
    using Func = std::function<void()>;
    virtual void runInLoop(const Func &func) = 0;
    virtual void runNext(const Func &func) = 0;
    virtual void run(const Func &func) = 0;
    /**
     * runInLoop(), runNext(), run() 区别
     *
     * runInLoop()
     *   功能：注入下一轮将执行的函数，有加锁操作，支持跨线程，跨Loop间调用；
     *   场景：常用于不同Loop之间委派任务或其它线程向Loop线程妥派任务。
     *
     * runNext()
     *   功能：注入本回调完成后立即执行的函数，无加锁操作，不支持跨线程与跨Loop间调用；
     *   场景：常用于不方便在本函数中执行的操作，比如释放对象自身。
     *   注意：不可以在 runNext() 执行函数中再 runNext()，否则会陷入死循环。
     *
     * runInLoop() 能替代 runNext()，但 runNext() 比 runInLoop() 更轻量。
     *
     * run()
     *   功能：自动选择 runNext() 或是 runInLoop()。
     *         当与Loop在同一线程时，选择 runNext()，否则选择 runInLoop()。
     *   场景：当不知道该怎么选择，只想让动作尽快被执行时。
     *   注意：同 runNext()
     *
     * 使用建议：
     *   能用 runNext() 的场景就用 runNext()，不能用再使用 runInLoop()。
     *   如果你嫌麻烦，就直接使用 run()。让它自己选择吧。
     */

    //! 创建事件
    virtual FdEvent* newFdEvent() = 0;
    virtual TimerEvent* newTimerEvent() = 0;
    virtual SignalEvent* newSignalEvent() = 0;

    //! 统计操作
    virtual void setStatEnable(bool enable) = 0;
    virtual Stat getStat() const = 0;
    virtual void resetStat() = 0;

  public:
    virtual ~Loop() { }
};

}
}

#endif //TBOX_EVENT_LOOP_H
