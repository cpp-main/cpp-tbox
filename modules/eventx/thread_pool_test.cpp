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

#include "thread_pool.h"

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

ThreadPool::TaskToken null_task_token;

/**
 * 最小线程数为2，最大线程数为5
 */
TEST(ThreadPool, min2_max5) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(2,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), null_task_token);
    }

    LogDbg("run in main");
    loop->exitLoop(std::chrono::seconds(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * 最小线程数为0，最大线程数为5
 */
TEST(ThreadPool, min0_max5) {
    Loop *loop = Loop::New();

    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(0,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), null_task_token);
    }

    LogDbg("run in main");
    loop->exitLoop(std::chrono::seconds(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * 不等其完成工作就退出主线程
 * 主要是检查有没有内存泄漏
 */
TEST(ThreadPool, exit_before_finish) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    for (int i = 0; i < 3; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), null_task_token);
    }

    LogDbg("run in main");
    loop->exitLoop(std::chrono::seconds(1));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * 取消任务
 *
 * 创建三个任务。在1.5秒时全部取消。
 */
TEST(ThreadPool, cancel_task) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    vector<ThreadPool::TaskToken> task_ids;
    for (int i = 0; i < 3; ++i) {
        auto token = tp->execute(std::bind(backend_func, i));
        task_ids.push_back(token);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    EXPECT_EQ(tp->cancel(task_ids[0]), 1);  //! 第一个任务已完成
    EXPECT_EQ(tp->cancel(task_ids[1]), 2);  //! 第二个任务正在执行
    EXPECT_EQ(tp->cancel(task_ids[2]), 0);  //! 第三个任务可正常取消
    ThreadPool::TaskToken invalid_token(100, 1);
    EXPECT_EQ(tp->cancel(invalid_token), 1);  //! 任务不存在

    loop->exitLoop(std::chrono::seconds(4));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * 优先级
 *
 * 期望优先级最高的先被执行
 */
TEST(ThreadPool, prio) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    vector<int> task_ids;
    auto backend_func = \
        [&task_ids](int id) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            LogDbg("task id: %d", id);
            task_ids.push_back(id);
        };

    tp->execute([] { std::this_thread::sleep_for(std::chrono::seconds(1)); });
    for (int i = 0; i < 5; ++i)
        tp->execute(std::bind(backend_func, i), 2-i);

    loop->exitLoop(std::chrono::seconds(2));
    loop->runLoop();

    ASSERT_EQ(task_ids.size(), 5);
    EXPECT_EQ(task_ids[0], 4);
    EXPECT_EQ(task_ids[1], 3);
    EXPECT_EQ(task_ids[2], 2);
    EXPECT_EQ(task_ids[3], 1);
    EXPECT_EQ(task_ids[4], 0);

    tp->cleanup();

    delete tp;
    delete loop;
}

/**
 * 优先级
 *
 * 期望优先级最高的先被执行
 */
TEST(ThreadPool, getStatus) {
    Loop *loop = Loop::New();

    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    auto task1 = tp->execute([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    auto task2 = tp->execute([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });

    auto t1 = loop->newTimerEvent();
    t1->initialize(chrono::milliseconds(50), Event::Mode::kOneshot);
    t1->enable();
    t1->setCallback(
        [&] {
            EXPECT_EQ(tp->getTaskStatus(task1), ThreadPool::TaskStatus::kExecuting);
            EXPECT_EQ(tp->getTaskStatus(task2), ThreadPool::TaskStatus::kWaiting);
        }
    );

    auto t2 = loop->newTimerEvent();
    t2->initialize(chrono::milliseconds(150), Event::Mode::kOneshot);
    t2->enable();
    t2->setCallback(
        [&] {
            EXPECT_EQ(tp->getTaskStatus(task1), ThreadPool::TaskStatus::kNotFound);
            EXPECT_EQ(tp->getTaskStatus(task2), ThreadPool::TaskStatus::kExecuting);
        }
    );

    auto t3 = loop->newTimerEvent();
    t3->initialize(chrono::milliseconds(250), Event::Mode::kOneshot);
    t3->enable();
    t3->setCallback(
        [&] {
            EXPECT_EQ(tp->getTaskStatus(task1), ThreadPool::TaskStatus::kNotFound);
            EXPECT_EQ(tp->getTaskStatus(task2), ThreadPool::TaskStatus::kNotFound);
        }
    );

    loop->exitLoop(std::chrono::milliseconds(300));
    loop->runLoop();

    tp->cleanup();

    delete t3;
    delete t2;
    delete t1;
    delete tp;
    delete loop;
}

/**
 * 在妥派的任务结束前执行cleanup()
 *
 * 注：之所以添加该单元测试，是因为在1.4.12之前存在线程多重join()的问题
 */
TEST(ThreadPool, cleanup_before_finish) {
    Loop *loop = Loop::New();
    ThreadPool *tp = new ThreadPool(loop);

    tp->initialize(0,1);
    loop->run(
        [=] {
            tp->execute([] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
        }
    );
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    tp->cleanup();

    delete tp;
    delete loop;
}

}
