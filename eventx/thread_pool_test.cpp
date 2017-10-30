#include <thread>
#include <gtest/gtest.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

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

/**
 * 最小线程数为2，最大线程数为5
 */
TEST(eventx_thread_pool, min2_max5) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(2,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
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
TEST(eventx_thread_pool, min0_max5) {
    Loop *loop = Loop::New();

    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(0,5));

    for (int i = 0; i < 12; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
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
TEST(eventx_thread_pool, exit_before_finish) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    for (int i = 0; i < 3; ++i) {
        ASSERT_NE(tp->execute(std::bind(backend_func, i)), -1);
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
TEST(eventx_thread_pool, cancel_task) {
    Loop *loop = Loop::New();

    LogDbg("%d", 123);
    ThreadPool *tp = new ThreadPool(loop);
    ASSERT_TRUE(tp->initialize(1,1));

    vector<int> task_ids;
    for (int i = 0; i < 3; ++i) {
        int tid = tp->execute(std::bind(backend_func, i));
        task_ids.push_back(tid);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    EXPECT_EQ(tp->cancel(task_ids[0]), 1);  //! 第一个任务已完成
    EXPECT_EQ(tp->cancel(task_ids[1]), 2);  //! 第二个任务正在执行
    EXPECT_EQ(tp->cancel(task_ids[2]), 0);  //! 第三个任务可正常取消
    EXPECT_EQ(tp->cancel(100), 1);  //! 任务不存在

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
TEST(eventx_thread_pool, prio) {
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

}
