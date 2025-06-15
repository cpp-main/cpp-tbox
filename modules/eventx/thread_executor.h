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
#ifndef TBOX_EVENTX_THREAD_EXECUTOR_H
#define TBOX_EVENTX_THREAD_EXECUTOR_H

#include <functional>
#include <tbox/base/cabinet_token.h>

namespace tbox {
namespace eventx {

/**
 * 线程执行器接口
 */
class ThreadExecutor {
  public:
    using TaskToken = cabinet::Token;
    using NonReturnFunc = std::function<void ()>;

    enum class TaskStatus {
        kWaiting,   //!< 等待中
        kExecuting, //!< 执行中
        kNotFound,  //!< 未找到（可能已完成）
        kCleanup,   //!< 执行器与被清除
    };

    enum class CancelResult {
        kSuccess,   //!< 成功
        kExecuting, //!< 该任务正在执行
        kNotFound,  //!< 没有找到该任务（可能已完成）
        kCleanup,   //!< 执行器与被清除
    };

  public:
    /**
     * 使用worker线程执行某个函数
     *
     * \param backend_task      让worker线程执行的函数对象
     *
     * \return TaskToken        任务Token
     */
    virtual TaskToken execute(NonReturnFunc &&backend_task) = 0;
    virtual TaskToken execute(const NonReturnFunc &backend_task) = 0;

    /**
     * 使用worker线程执行某个函数，并在完成之后在主线程执行指定的回调函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param main_cb           任务完成后，由主线程执行的回调函数对象
     * \param prio              任务优先级[-2, -1, 0, 1, 2]，越小优先级越高
     *
     * \return TaskToken        任务Token
     */
    virtual TaskToken execute(NonReturnFunc &&backend_task, NonReturnFunc &&main_cb) = 0;
    virtual TaskToken execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb) = 0;


    //! 获取任务的状态
    virtual TaskStatus getTaskStatus(TaskToken task_token) const = 0;

    //! 取消任务
    virtual CancelResult cancel(TaskToken task_token) = 0;
};

}
}

#endif //TBOX_EVENTX_THREAD_EXECUTOR_H
