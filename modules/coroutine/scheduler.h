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
#ifndef TBOX_COROUTINE_SCHEDULER_H_20180519
#define TBOX_COROUTINE_SCHEDULER_H_20180519

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace coroutine {

struct Routine;
class Scheduler;

using RoutineToken  = cabinet::Token;
using RoutineEntry  = std::function<void(Scheduler&)>;

#define ROUTINE_STACK_DEFAULT_SIZE  8192    //! 子协程默认栈大小

//! 协程调度器
class Scheduler {
    friend struct Routine;

  public:
    explicit Scheduler(event::Loop *wp_loop);
    virtual ~Scheduler();

    NONCOPYABLE(Scheduler);
    IMMOVABLE(Scheduler);

  public:
    //! 创建一个协程，并返回协程Token。创建后不自动执行，需要一次 resume()
    RoutineToken create(const RoutineEntry &entry,
                        bool run_now = true,            //! 是否立即运行
                        const std::string &name = "",   //! 子协程名
                        size_t stack_size = ROUTINE_STACK_DEFAULT_SIZE);

    bool resume(const RoutineToken &token); //! 恢复指定协程
    bool cancel(const RoutineToken &token); //! 取消指定协程，只能给协程发送了取消请求，并非立即停止

  public:
    //! 以下仅限子协程调用
    void wait();    //! 切换到主协程，等待被 resumeRoutine() 唤醒
    void yield();   //! 切换到主协程，等待下一个事件循环继续执行
    bool join(const RoutineToken &other_routine);   //! 一个协程等待另一个协程结束

    RoutineToken getToken() const;  //! 获取当前协程token
    bool isCanceled() const;        //! 当前协程是否被取消
    std::string getName() const;    //! 当前协程的名称
    event::Loop* getLoop() const;

  public:
    //! 以下仅限主协程调用
    void cleanup(); //! 强行停止并清理所有的协程，通常在程序退出前使用

  protected:
    void schedule();    //! 调度，依次切换到已就绪的 Routine 去执行，直到没有 Routine 就绪为止

    bool makeRoutineReady(Routine *routine);
    void switchToRoutine(Routine *routine);
    bool isInMainRoutine() const;   //! 是否处于主协程中

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_COROUTINE_SCHEDULER_H_20180519
