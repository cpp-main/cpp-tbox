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
#ifndef TBOX_WORK_THREAD_H
#define TBOX_WORK_THREAD_H

#include <limits>
#include <functional>
#include <array>
#include <tbox/event/forward.h>
#include <tbox/base/cabinet_token.h>

#include "thread_executor.h"

namespace tbox {
namespace eventx {

/**
 * 工作线程
 *
 * 是ThreadPool的精简版，单线程+无任务等级
 */
class WorkThread : public ThreadExecutor {
  public:
    /**
     * 构造函数
     *
     * \param main_loop         主线程的Loop对象指针
     */
    explicit WorkThread(event::Loop *main_loop = nullptr);
    virtual ~WorkThread();

    TaskToken execute(NonReturnFunc &&backend_task, NonReturnFunc &&main_cb, event::Loop *main_loop);
    TaskToken execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, event::Loop *main_loop);

    /**
     * 使用worker线程执行某个函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param main_cb           任务完成后，由主线程执行的回调函数对象
     *
     * \return TaskToken        任务Token
     */
    virtual TaskToken execute(NonReturnFunc &&backend_task) override;
    virtual TaskToken execute(const NonReturnFunc &backend_task) override;
    virtual TaskToken execute(NonReturnFunc &&backend_task, NonReturnFunc &&main_cb) override;
    virtual TaskToken execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb) override;

    //! 获取任务的状态
    virtual TaskStatus getTaskStatus(TaskToken task_token) const override;
    //! 取消任务
    virtual CancelResult cancel(TaskToken task_token) override;

    /**
     * 清理资源，并等待线程结束
     */
    void cleanup();

  protected:
    void threadProc();

    bool shouldThreadExitWaiting() const;   //! 判定子线程是否需要退出条件变量的wait()函数

    struct Task;
    Task* popOneTask(); //! 取出一个优先级最高的任务

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_WORK_THREAD_H
