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
#include "work_thread.h"

#include <cinttypes>
#include <set>
#include <deque>
#include <thread>
#include <mutex>
#include <algorithm>
#include <condition_variable>
#include <chrono>

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/base/object_pool.hpp>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

using Clock = std::chrono::steady_clock;

//! WorkThread 的私有数据
struct WorkThread::Data {
    event::Loop *default_main_loop = nullptr; //!< 主线程

    std::mutex lock;                    //!< 互斥锁
    std::condition_variable cond_var;   //!< 条件变量

    std::thread work_thread;

    cabinet::Cabinet<Task> undo_tasks_cabinet;

    std::deque<TaskToken> undo_tasks_token_deque;   //!< 排队中的任务队列
    std::set<TaskToken> doing_tasks_token;          //!< 正在处理的任务集合

    ObjectPool<Task> task_pool{64};

    bool stop_flag = false; //!< 是否立即停止标记
};

/**
 * 任务项
 */
struct WorkThread::Task {
    TaskToken token;
    NonReturnFunc backend_task;   //! 任务在工作线程中执行函数
    NonReturnFunc main_cb;        //! 任务执行完成后由main_loop执行的回调函数
    event::Loop  *main_loop = nullptr;
    Clock::time_point create_time_point;

    Task *next = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////

WorkThread::WorkThread(event::Loop *main_loop) :
    d_(new Data)
{
    d_->default_main_loop = main_loop;
    d_->work_thread = std::thread(std::bind(&WorkThread::threadProc, this));
    d_->stop_flag = false;
}

WorkThread::~WorkThread()
{
    cleanup();
}

WorkThread::TaskToken WorkThread::execute(NonReturnFunc &&backend_task)
{
    return execute(std::move(backend_task), nullptr, nullptr);
}

WorkThread::TaskToken WorkThread::execute(const NonReturnFunc &backend_task)
{
    NonReturnFunc backend_task_copy(backend_task);
    return execute(std::move(backend_task_copy), nullptr, nullptr);
}

WorkThread::TaskToken WorkThread::execute(NonReturnFunc &&backend_task, NonReturnFunc &&main_cb, event::Loop *main_loop)
{
    RECORD_SCOPE();
    TaskToken token;

    if (d_ == nullptr) {
        LogWarn("WorkThread has been cleanup");
        return token;
    }

    {
        std::lock_guard<std::mutex> lg(d_->lock);

        Task *item = d_->task_pool.alloc();
        item->backend_task = std::move(backend_task);
        item->main_cb = std::move(main_cb);
        item->main_loop = (main_loop != nullptr) ? main_loop : d_->default_main_loop;
        item->create_time_point = Clock::now();
        item->token = token = d_->undo_tasks_cabinet.alloc(item);

        d_->undo_tasks_token_deque.push_back(token);
    }

    LogDbg("create task %u", token.id());
    d_->cond_var.notify_one();

    return token;
}

WorkThread::TaskToken WorkThread::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, event::Loop *main_loop)
{
    NonReturnFunc backend_task_copy(backend_task);
    NonReturnFunc main_cb_copy(main_cb);
    return execute(std::move(backend_task_copy), std::move(main_cb_copy), main_loop);
}

WorkThread::TaskStatus WorkThread::getTaskStatus(TaskToken task_token) const
{
    if (d_ == nullptr) {
        LogWarn("WorkThread has been cleanup");
        return TaskStatus::kNotFound;
    }

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
int WorkThread::cancel(TaskToken token)
{
    RECORD_SCOPE();
    if (d_ == nullptr) {
        LogWarn("WorkThread has been cleanup");
        return 3;
    }

    std::lock_guard<std::mutex> lg(d_->lock);

    //! 如果正在执行
    if (d_->doing_tasks_token.find(token) != d_->doing_tasks_token.end())
        return 2;   //! 返回正在执行

    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    if (!d_->undo_tasks_token_deque.empty()) {
        auto iter = std::find(d_->undo_tasks_token_deque.begin(), d_->undo_tasks_token_deque.end(), token);
        if (iter != d_->undo_tasks_token_deque.end()) {
            d_->undo_tasks_token_deque.erase(iter);
            d_->task_pool.free(d_->undo_tasks_cabinet.free(token));
            return 0;
        }
    }

    return 1;   //! 返回没有找到
}

void WorkThread::threadProc()
{
    LogDbg("thread start");

    while (true) {
        Task* item = nullptr;
        {
            std::unique_lock<std::mutex> lk(d_->lock);

            //! 等待任务
            d_->cond_var.wait(lk, std::bind(&WorkThread::shouldThreadExitWaiting, this));

            /**
             * 有两种情况会从 cond_var.wait() 退出
             * 1. 任务队列中有任务需要执行时
             * 2. 析构时要求所有工作线程退出时
             *
             * 所以，下面检查 stop_flag 看是不是请求退出
             */
            if (d_->stop_flag) {
                LogDbg("thread will exit, stop flag.");
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

            LogDbg("thread pick task %u", item->token.id());

            auto exec_time_point = Clock::now();
            auto wait_time_cost = exec_time_point - item->create_time_point;

            {
                RECORD_SCOPE();
                CatchThrow(item->backend_task, true);
            }

            auto exec_time_cost = Clock::now() - exec_time_point;

            LogDbg("thread finish task %u, cost %" PRIu64 " + %" PRIu64 " us",
                   item->token.id(),
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
            auto main_loop = item->main_loop;

            {
                std::lock_guard<std::mutex> lg(d_->lock);
                d_->doing_tasks_token.erase(item->token);
                d_->task_pool.free(item);
            }

            if (main_cb && main_loop != nullptr) {
                RECORD_SCOPE();
                main_loop->runInLoop(std::move(main_cb), "WorkThread::threadProc, invoke main_cb");
            }
        }
    }

    LogDbg("thread exit");
}

bool WorkThread::shouldThreadExitWaiting() const
{
    return d_->stop_flag || !d_->undo_tasks_token_deque.empty();
}

WorkThread::Task* WorkThread::popOneTask()
{
    if (!d_->undo_tasks_token_deque.empty()) {
        TaskToken token = d_->undo_tasks_token_deque.front();
        d_->undo_tasks_token_deque.pop_front();
        return d_->undo_tasks_cabinet.free(token);
    }
    return nullptr;
}

void WorkThread::cleanup()
{
    if (d_ == nullptr)
        return;

    {
        std::lock_guard<std::mutex> lg(d_->lock);
        //! 清空task中的任务
        while (!d_->undo_tasks_token_deque.empty()) {
            auto token = d_->undo_tasks_token_deque.front();
            d_->task_pool.free(d_->undo_tasks_cabinet.free(token));
            d_->undo_tasks_token_deque.pop_front();
        }
    }

    d_->stop_flag = true;
    d_->cond_var.notify_all();

    d_->work_thread.join();

    CHECK_DELETE_RESET_OBJ(d_);
}

}
}
