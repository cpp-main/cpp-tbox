/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_EVENT_LOOP_H
#define TBOX_EVENT_LOOP_H

#include <functional>
#include <chrono>
#include <string>
#include <vector>

#include "forward.h"
#include "stat.h"

namespace tbox {
namespace event {

class Loop {
  public:
    //! 创建默认类型的事件循环
    static Loop* New();
    //! 创建指定类型的事件循环
    static Loop* New(const std::string &engine_type);
    //! 获取引擎列表
    static std::vector<std::string> Engines();

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


    //! 委托与取消委托延后执行动作
    using RunId = uint64_t;
    using Func = std::function<void()>;
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
     *   注意：仅Loop线程中调用，禁止跨线程操作
     *
     * runInLoop() 能替代 runNext()，但 runNext() 比 runInLoop() 更高效。
     *
     * run()
     *   功能：自动选择 runNext() 或是 runInLoop()。
     *         当与Loop在同一线程时，选择 runNext()，否则选择 runInLoop()。
     *   场景：当不知道该怎么选择，选它就对了。
     *
     * 使用建议：
     *   明确是Loop线程内的用 runNext(), 明确不是Loop线程内的用 runInLoop()。
     *   不清楚的直接用 run()。
     */
    virtual RunId runInLoop(Func &&func, const std::string &what = "") = 0;
    virtual RunId runInLoop(const Func &func, const std::string &what = "") = 0;
    virtual RunId runNext(Func &&func, const std::string &what = "") = 0;
    virtual RunId runNext(const Func &func, const std::string &what = "") = 0;
    virtual RunId run(Func &&func, const std::string &what = "") = 0;
    virtual RunId run(const Func &func, const std::string &what = "") = 0;
    virtual bool  cancel(RunId run_id) = 0;

    //! 创建事件
    virtual FdEvent* newFdEvent(const std::string &what = "") = 0;
    virtual TimerEvent* newTimerEvent(const std::string &what = "") = 0;
    virtual SignalEvent* newSignalEvent(const std::string &what = "") = 0;

    //! 统计
    virtual Stat getStat() const = 0;
    virtual void resetStat() = 0;

    //! 阈值
    struct WaterLine {
      size_t run_in_loop_queue_size;              //!< runInLoop() 队列长度
      size_t run_next_queue_size;                 //!< runNext() 队列长度
      std::chrono::nanoseconds wake_delay;        //!< loop 唤醒延迟
      std::chrono::nanoseconds loop_cost;         //!< loop 时间消耗
      std::chrono::nanoseconds event_cb_cost;     //!< 事件回调时间消耗
      std::chrono::nanoseconds run_cb_cost;       //!< Run回调时间消耗
      std::chrono::nanoseconds run_in_loop_delay; //!< runInLoop() 执行延迟
      std::chrono::nanoseconds run_next_delay;    //!< runNext() 执行延迟
      std::chrono::nanoseconds timer_delay;       //!< 定时器延迟
    };
    virtual WaterLine& water_line() = 0;

    virtual void cleanup() = 0;

  public:
    virtual ~Loop() { }
};

}
}

#endif //TBOX_EVENT_LOOP_H
