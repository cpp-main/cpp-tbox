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
#include <thread>
#include <gtest/gtest.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

#include "work_thread.h"

using namespace std;
using namespace tbox::event;
using namespace tbox::eventx;

namespace {

auto backend_func = \
        [](int id) {
            LogDbg("<<task %d run in back", id);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            LogDbg("task %d run in back>>", id);
        };

WorkThread::TaskToken null_task_token;

/**
 * 测试是否所有的任务都能被执行
 */
TEST(WorkThread, all_task_executed_without_loop) {
    WorkThread *tp = new WorkThread;

    int count = 0;
    for (int i = 0; i < 3; ++i) {
        tp->execute([&] {++count;});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    delete tp;
    EXPECT_EQ(count, 3);
}

/**
 * 测试是否所有的任务都能被执行
 */
TEST(WorkThread, all_task_executed_with_loop) {
    Loop *loop = Loop::New();

    WorkThread *tp = new WorkThread(loop);

    int count = 0;
    for (int i = 0; i < 3; ++i) {
        tp->execute([]{}, [&]{++count;});
    }

    LogDbg("run in main");
    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    delete tp;
    delete loop;

    EXPECT_EQ(count, 3);
}

/**
 * 不等其完成工作就退出主线程
 * 主要是检查有没有内存泄漏
 */
TEST(WorkThread, exit_before_finish) {
    Loop *loop = Loop::New();

    WorkThread *tp = new WorkThread(loop);

    for (int i = 0; i < 3; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), null_task_token);
    }

    LogDbg("run in main");
    loop->exitLoop(std::chrono::seconds(1));
    loop->runLoop();

    delete tp;
    delete loop;
}

/**
 * 取消任务
 *
 * 创建三个任务。在1.5秒时全部取消。
 */
TEST(WorkThread, cancel_task) {
    Loop *loop = Loop::New();

    WorkThread *tp = new WorkThread(loop);

    vector<WorkThread::TaskToken> task_ids;
    for (int i = 0; i < 3; ++i) {
        auto token = tp->execute(std::bind(backend_func, i));
        task_ids.push_back(token);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    EXPECT_EQ(tp->cancel(task_ids[0]), 1);  //! 第一个任务已完成
    EXPECT_EQ(tp->cancel(task_ids[1]), 2);  //! 第二个任务正在执行
    EXPECT_EQ(tp->cancel(task_ids[2]), 0);  //! 第三个任务可正常取消
    WorkThread::TaskToken invalid_token(100, 1);
    EXPECT_EQ(tp->cancel(invalid_token), 1);  //! 任务不存在

    loop->exitLoop(std::chrono::seconds(4));
    loop->runLoop();

    delete tp;
    delete loop;
}

}
