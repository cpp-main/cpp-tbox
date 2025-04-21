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
#include "thread_pool.h"

#include <cinttypes>
#include <array>
#include <map>
#include <set>
#include <deque>
#include <thread>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <chrono>

#include <tbox/base/log.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/base/assert.h>
#include <tbox/base/catch_throw.h>
#include <tbox/base/object_pool.hpp>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

using Clock = std::chrono::steady_clock;

//! ThreadPool 的私有数据
struct ThreadPool::Data {
    event::Loop *wp_loop = nullptr; //!< 主线程

    bool is_ready = false;     //! 是否已经初始化了

    size_t min_thread_num = 0; //!< 最少的线程个数
    size_t max_thread_num = 0; //!< 最多的线程个数

    std::mutex lock;                //!< 互斥锁
    std::condition_variable cond_var;   //!< 条件变量

    cabinet::Cabinet<Task> undo_tasks_cabinet;
    std::array<std::deque<TaskToken>, THREAD_POOL_PRIO_SIZE> undo_tasks_token; //!< 优先级任务列表，THREAD_POOL_PRIO_SIZE级
    std::set<TaskToken> doing_tasks_token;    //!< 记录正在从事的任务

    size_t idle_thread_num = 0;         //!< 空间线程个数
    cabinet::Cabinet<std::thread> threads_cabinet;
    bool all_threads_stop_flag = false; //!< 是否所有工作线程立即停止标记

    size_t undo_task_peak_num_ = 0;

    ObjectPool<Task> task_pool{64};
};

/**
 * 任务项
 */
struct ThreadPool::Task {
    TaskToken token;
    NonReturnFunc backend_task;   //! 任务在工作线程中执行函数
    NonReturnFunc main_cb;        //! 任务执行完成后由main_loop执行的回调函数
    Clock::time_point create_time_point;

    Task *next = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(event::Loop *main_loop) :
    d_(new Data)
{
    d_->wp_loop = main_loop;
}

ThreadPool::~ThreadPool()
{
    if (d_->is_ready)
        cleanup();

    delete d_;
}

bool ThreadPool::initialize(ssize_t min_thread_num, ssize_t max_thread_num)
{
    if (d_->is_ready) {
        LogWarn("it has ready, cleanup() first");
        return false;
    }

    if (max_thread_num < 0 || min_thread_num < 0 ||
            min_thread_num > max_thread_num || max_thread_num == 0) {

        LogWarn("min_thread_num or max_thread_num invalid, min:%d, max:%d", min_thread_num, max_thread_num);
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(d_->lock);
        d_->min_thread_num = min_thread_num;
        d_->max_thread_num = max_thread_num;

        for (ssize_t i = 0; i < min_thread_num; ++i)
            if (!createWorker())
                return false;
    }

    d_->all_threads_stop_flag = false;
    d_->is_ready = true;

    return true;
}

ThreadPool::TaskToken ThreadPool::execute(NonReturnFunc &&backend_task, int prio)
{
    return execute(std::move(backend_task), nullptr, prio);
}

ThreadPool::TaskToken ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return execute(backend_task, nullptr, prio);
}

ThreadPool::TaskToken ThreadPool::execute(NonReturnFunc &&backend_task, NonReturnFunc &&main_cb, int prio)
{
    RECORD_SCOPE();
    TaskToken token;

    if (!d_->is_ready) {
        LogWarn("need initialize() first");
        return token;
    }

    if (prio < THREAD_POOL_PRIO_MIN)
        prio = THREAD_POOL_PRIO_MIN;
    else if (prio > THREAD_POOL_PRIO_MAX)
        prio = THREAD_POOL_PRIO_MAX;

    int level = prio + THREAD_POOL_PRIO_MAX;

    {
        std::lock_guard<std::mutex> lg(d_->lock);

        Task *item = d_->task_pool.alloc();
        item->backend_task = std::move(backend_task);
        item->main_cb = std::move(main_cb);
        item->create_time_point = Clock::now();
        item->token = token = d_->undo_tasks_cabinet.alloc(item);

        d_->undo_tasks_token.at(level).push_back(token);
        //! 如果空闲线程不够分配未认领的任务，且还可以再创建新的线程
        if (d_->undo_tasks_cabinet.size() > d_->idle_thread_num) {
            if (d_->threads_cabinet.size() < d_->max_thread_num) {
                createWorker();
            } else {
                if (d_->undo_task_peak_num_ < d_->undo_tasks_cabinet.size())
                    d_->undo_task_peak_num_ = d_->undo_tasks_cabinet.size();
            }
        }
    }

    LogDbg("create task %u", token.id());
    d_->cond_var.notify_one();

    return token;
}

ThreadPool::TaskToken ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    NonReturnFunc backend_task_copy(backend_task);
    NonReturnFunc main_cb_copy(main_cb);
    return execute(std::move(backend_task_copy), std::move(main_cb_copy), prio);
}

ThreadPool::TaskStatus ThreadPool::getTaskStatus(TaskToken task_token) const
{
    std::lock_guard<std::mutex> lg(d_->lock);

    if (d_->undo_tasks_cabinet.at(task_token) != nullptr)
        return TaskStatus::kWaiting;

    if (d_->doing_tasks_token.find(task_token) != d_->doing_tasks_token.end())
        return TaskStatus::kExecuting;

    return TaskStatus::kNotFound;
}

/**
 * 返回值如下：
 * 0: 取消成功
 * 1: 没有找到该任务
 * 2: 该任务正在执行
 */
int ThreadPool::cancel(TaskToken token)
{
    RECORD_SCOPE();
    std::lock_guard<std::mutex> lg(d_->lock);

    //! 如果正在执行
    if (d_->doing_tasks_token.find(token) != d_->doing_tasks_token.end())
        return 2;   //! 返回正在执行

    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < d_->undo_tasks_token.size(); ++i) {
        auto &tasks_token = d_->undo_tasks_token.at(i);
        if (!tasks_token.empty()) {
            auto iter = std::find(tasks_token.begin(), tasks_token.end(), token);
            if (iter != tasks_token.end()) {
                tasks_token.erase(iter);
                d_->task_pool.free(d_->undo_tasks_cabinet.free(token));
                return 0;
            }
        }
    }

    return 1;   //! 返回没有找到
}

void ThreadPool::cleanup()
{
    RECORD_SCOPE();
    if (!d_->is_ready)
        return;

    std::vector<std::thread*> thread_vec;
    {
        std::lock_guard<std::mutex> lg(d_->lock);
        //! 清空task中的任务
        for (size_t i = 0; i < d_->undo_tasks_token.size(); ++i) {
            auto &tasks_token = d_->undo_tasks_token.at(i);
            while (!tasks_token.empty()) {
                auto token = tasks_token.front();
                d_->task_pool.free(d_->undo_tasks_cabinet.free(token));
                tasks_token.pop_front();
            }
        }

        //! 将threads_cabinet中的线程搬到thread_vec
        thread_vec.reserve(d_->threads_cabinet.size());
        d_->threads_cabinet.foreach(
            [&](std::thread *t) {
                thread_vec.push_back(t);
            }
        );
        d_->threads_cabinet.clear();
    }

    d_->all_threads_stop_flag = true;
    d_->cond_var.notify_all();

    //! 等待所有的线程退出
    for (auto t : thread_vec) {
        t->join();
        delete t;
    }

    d_->is_ready = false;
}

ThreadPool::Snapshot ThreadPool::snapshot() const
{
    Snapshot ss;
    std::lock_guard<std::mutex> lg(d_->lock);

    ss.idle_thread_num = d_->idle_thread_num;
    ss.thread_num = d_->threads_cabinet.size();
    ss.doing_task_num = d_->doing_tasks_token.size();
    for (size_t i = 0; i < THREAD_POOL_PRIO_SIZE; ++i)
        ss.undo_task_num[i] = d_->undo_tasks_token[i].size();
    ss.undo_task_peak_num = d_->undo_task_peak_num_;

    return ss;
}

void ThreadPool::threadProc(ThreadToken thread_token)
{
    bool let_main_loop_join_me = false;

    LogDbg("thread %u start", thread_token.id());

    while (true) {
        Task* item = nullptr;
        {
            std::unique_lock<std::mutex> lk(d_->lock);

            /**
             * 如果当前空闲的线程数量大于等于未被领取的任务数，且当前的线程个数已超过长驻线程数，
             * 说明线程数据已满足现有要求则退出当前线程
             */
            if ((d_->idle_thread_num >= d_->undo_tasks_cabinet.size()) && (d_->threads_cabinet.size() > d_->min_thread_num)) {
                LogDbg("thread %u will exit, no more work.", thread_token.id());
                let_main_loop_join_me = true;
                break;
            }

            //! 等待任务
            ++d_->idle_thread_num;
            d_->cond_var.wait(lk, std::bind(&ThreadPool::shouldThreadExitWaiting, this));
            --d_->idle_thread_num;

            /**
             * 有两种情况会从 cond_var.wait() 退出
             * 1. 任务队列中有任务需要执行时
             * 2. 线程池 cleanup() 时要求所有工作线程退出时
             *
             * 所以，下面检查 all_threads_stop_flag 看是不是请求退出
             */
            if (d_->all_threads_stop_flag) {
                LogDbg("thread %u will exit, stop flag.", thread_token.id());
                break;
            }

            item = popOneTask();    //! 从任务队列中取出优先级最高的任务
        }

        //! 后面就是去执行任务，不需要再加锁了
        if (item != nullptr) {
            RECORD_SCOPE();
            {
                std::lock_guard<std::mutex> lg(d_->lock);
                d_->doing_tasks_token.insert(item->token);
            }

            LogDbg("thread %u pick task %u", thread_token.id(), item->token.id());

            auto exec_time_point = Clock::now();
            auto wait_time_cost = exec_time_point - item->create_time_point;

            {
                RECORD_SCOPE();
                CatchThrow(item->backend_task, true);
            }

            auto exec_time_cost = Clock::now() - exec_time_point;

            LogDbg("thread %u finish task %u, cost %" PRIu64 " + %" PRIu64 " us",
                   thread_token.id(), item->token.id(),
                   wait_time_cost.count() / 1000,
                   exec_time_cost.count() / 1000);

            /**
             * 有时在妥托给WorkThread执行动作时，会在lamda中捕获智能指针，它所指向的
             * 对象的析构函数是有动作的，如：http的sp_ctx要在析构中发送HTTP回复，如果
             * 析构函数在子线程中执行，则会出现不希望见到的多线程竞争。为此，我们在main_cb
             * 中也让它持有这个智能指针，希望智能指针所指的对象只在主线程中析构。
             *
             * 为了保证main_cb中的持有的对象能够在main_loop线程中被析构，
             * 所以这里要先task_pool.free()，然后再runInLoop(std::move(main_cpp))
             */

            auto main_cb = std::move(item->main_cb);

            {
                std::lock_guard<std::mutex> lg(d_->lock);
                d_->doing_tasks_token.erase(item->token);
                d_->task_pool.free(item);
            }

            if (main_cb) {
                RECORD_SCOPE();
                d_->wp_loop->runInLoop(std::move(main_cb), "ThreadPool::threadProc, invoke main_cb");
            }
        }
    }

    LogDbg("thread %u exit", thread_token.id());

    if (let_main_loop_join_me) {
        //! 则将线程取出来，交给main_loop去join()，然后delete
        std::unique_lock<std::mutex> lk(d_->lock);

        auto t = d_->threads_cabinet.free(thread_token);
        TBOX_ASSERT(t != nullptr);
        d_->wp_loop->runInLoop(
            [t]{ t->join(); delete t; },
            "ThreadPool::threadProc, join and delete it"
        );
        //! 这个操作放到最后来做是为了减少主线程join()的等待时长
    }
}

bool ThreadPool::createWorker()
{
    RECORD_SCOPE();
    ThreadToken thread_token = d_->threads_cabinet.alloc();
    auto *new_thread = new std::thread(std::bind(&ThreadPool::threadProc, this, thread_token));
    if (new_thread != nullptr) {
        d_->threads_cabinet.update(thread_token, new_thread);
        LogDbg("create thread %u", thread_token.id());
        return true;

    } else {
        LogErr("new thread fail");
        return false;
    }
}

bool ThreadPool::shouldThreadExitWaiting() const
{
    if (d_->all_threads_stop_flag)
        return true;

    for (size_t i = 0; i < d_->undo_tasks_token.size(); ++i) {
        const auto &tasks_token = d_->undo_tasks_token.at(i);
        if (!tasks_token.empty()) {
            return true;
        }
    }

    return false;
}

ThreadPool::Task* ThreadPool::popOneTask()
{
    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < d_->undo_tasks_token.size(); ++i) {
        auto &tasks_token = d_->undo_tasks_token.at(i);
        if (!tasks_token.empty()) {
            TaskToken token = tasks_token.front();
            tasks_token.pop_front();
            return d_->undo_tasks_cabinet.free(token);
        }
    }
    return nullptr;
}

}
}
