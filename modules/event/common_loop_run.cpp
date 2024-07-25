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
#include "common_loop.h"

#include <unistd.h>
#include <inttypes.h>
#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>

namespace tbox {
namespace event {

using namespace std::chrono;

CommonLoop::RunFuncItem::RunFuncItem(RunId i, Func &&f, const std::string &w)
    : id(i)
    , commit_time_point(steady_clock::now())
    , func(std::move(f))
    , what(w)
{ }

Loop::RunId CommonLoop::allocRunInLoopId()
{
    run_in_loop_id_alloc_ += 2;
    if (run_in_loop_id_alloc_ == 0) //! 确保分配到的ID一定不为0
      run_in_loop_id_alloc_ += 2;

    return run_in_loop_id_alloc_;
}

Loop::RunId CommonLoop::allocRunNextId()
{
    run_next_id_alloc_ += 2;
    return run_next_id_alloc_;
}

Loop::RunId CommonLoop::runInLoop(Func &&func, const std::string &what)
{
    RECORD_SCOPE();
    std::lock_guard<std::recursive_mutex> g(lock_);

    RunId run_id = allocRunInLoopId();
    run_in_loop_func_queue_.emplace_back(RunFuncItem(run_id, std::move(func), what));

    if (sp_run_read_event_ != nullptr)
        commitRunRequest();

    auto queue_size = run_in_loop_func_queue_.size();
    if (queue_size > water_line_.run_in_loop_queue_size)
        LogNotice("run_in_loop_queue_size: %u", queue_size);

    if (queue_size > run_in_loop_peak_num_)
        run_in_loop_peak_num_ = queue_size;

    return run_id;
}

Loop::RunId CommonLoop::runInLoop(const Func &func, const std::string &what)
{
    Func func_copy(func);
    return runInLoop(std::move(func_copy), what);
}

Loop::RunId CommonLoop::runNext(Func &&func, const std::string &what)
{
    RECORD_SCOPE();
    RunId run_id = allocRunNextId();
    run_next_func_queue_.emplace_back(RunFuncItem(run_id, std::move(func), what));

    auto queue_size = run_next_func_queue_.size();
    if (queue_size > water_line_.run_next_queue_size)
        LogNotice("run_next_queue_size: %u", queue_size);

    if (queue_size > run_next_peak_num_)
        run_next_peak_num_ = queue_size;

    return run_id;
}

Loop::RunId CommonLoop::runNext(const Func &func, const std::string &what)
{
    Func func_copy(func);
    return runNext(std::move(func_copy), what);
}

Loop::RunId CommonLoop::run(Func &&func, const std::string &what)
{
    RECORD_SCOPE();
    bool can_run_next = true;
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        if (isRunningLockless() && !isInLoopThreadLockless())
            can_run_next = false;
    }

    if (can_run_next)
        return runNext(std::move(func), what);
    else
        return runInLoop(std::move(func), what);
}

Loop::RunId CommonLoop::run(const Func &func, const std::string &what)
{
    Func func_copy(func);
    return run(std::move(func_copy), what);
}

//! 从队列中删除指定run_id的项
bool CommonLoop::RemoveRunFuncItemById(RunFuncQueue &run_deqeue, RunId run_id)
{
    auto is_run_id_match = [run_id] (RunFuncItem &item) { return item.id == run_id; };

    auto end_iter = run_deqeue.end();
    auto iter = std::remove_if(run_deqeue.begin(), end_iter, is_run_id_match);
    if (iter != end_iter) {
        run_deqeue.erase(iter, end_iter);
        return true;
    }
    return false;
};

bool CommonLoop::cancel(RunId run_id)
{
    if (run_id == 0)
        return false;

    //! 先从正在执行的任务队列里删
    if (RemoveRunFuncItemById(tmp_func_queue_, run_id))
        return true;

    if (run_id & 1) {   //! 奇数为runNext()的任务
        return RemoveRunFuncItemById(run_next_func_queue_, run_id);
    } else {    //! 偶数为runInLoop()的任务
        std::lock_guard<std::recursive_mutex> g(lock_);
        return RemoveRunFuncItemById(run_in_loop_func_queue_, run_id);
    }
}

void CommonLoop::handleNextFunc()
{
    RECORD_SCOPE();
    /**
     * 这里使用 tmp_func_queue_ 将 run_next_func_queue_ 中的内容交换出去。然后再从
     * tmp_func_queue_ 逐一取任务出来执行。其目的在于腾空 run_next_func_queue_，让
     * 新 runNext() 的任务则会在下一轮循环中执行。从而防止无限 runNext() 引起的死循
     * 环，导致其它事件得不到处理。
     *
     * 为什么tmp_func_queue_被定义成成员变量，而不是栈上的临时变量呢？
     * 如果使用的是临时变量，那么run_next_func_queue_被swap()之后任务，就无法被cancel
     * 这不符合cancel的功能要求。于是将其作为成员变量，方便cancel()的时候，被swap
     * 的任务也可以被cancel。
     */
    run_next_func_queue_.swap(tmp_func_queue_);

    while (!tmp_func_queue_.empty()) {
        auto item = tmp_func_queue_.front();
        tmp_func_queue_.pop_front();

        auto now = steady_clock::now();
        auto delay = now - item.commit_time_point;
        if (delay > water_line_.run_next_delay)
            LogNotice("run_next_delay: %" PRIu64 " us, what: '%s'",
                      delay.count()/1000, item.what.c_str());

        if (item.func) {
            RECORD_SCOPE();
            ++cb_level_;
            item.func();
            --cb_level_;
        }

        auto cost = steady_clock::now() - now;
        if (cost > water_line_.run_cb_cost)
            LogNotice("run_cb_cost: %" PRIu64 " us, what: '%s'",
                      cost.count()/1000, item.what.c_str());
    }
}

bool CommonLoop::hasNextFunc() const
{
    return !run_next_func_queue_.empty();
}

void CommonLoop::handleRunInLoopFunc()
{
    RECORD_SCOPE();
    {
        //! 同handleNextFunc()的说明
        std::lock_guard<std::recursive_mutex> g(lock_);
        run_in_loop_func_queue_.swap(tmp_func_queue_);
        finishRunRequest();
    }

    while (!tmp_func_queue_.empty()) {
        auto item = tmp_func_queue_.front();
        tmp_func_queue_.pop_front();

        auto now = steady_clock::now();
        auto delay = now - item.commit_time_point;
        if (delay > water_line_.run_in_loop_delay)
            LogNotice("run_in_loop_delay: %" PRIu64 " us, what: '%s'",
                      delay.count()/1000, item.what.c_str());

        if (item.func) {
            RECORD_SCOPE();
            ++cb_level_;
            item.func();
            --cb_level_;
        }

        auto cost = steady_clock::now() - now;
        if (cost > water_line_.run_cb_cost)
            LogNotice("run_cb_cost: %" PRIu64 " us, what: '%s'",
                      cost.count()/1000, item.what.c_str());
    }
}

//! 清理 run_in_loop_func_queue_ 与 run_next_func_queue_ 中的任务
void CommonLoop::cleanupDeferredTasks()
{
    int remain_loop_count = 100; //! 限定次数，防止出现 runNext() 递归导致无法退出循环的问题
    while ((!run_in_loop_func_queue_.empty() || !run_next_func_queue_.empty()) && remain_loop_count-- > 0) {

        RunFuncQueue run_next_tasks = std::move(run_next_func_queue_);
        RunFuncQueue run_in_loop_tasks = std::move(run_in_loop_func_queue_);

        while (!run_next_tasks.empty()) {
            RunFuncItem &item = run_next_tasks.front();
            if (item.func) {
                RECORD_SCOPE();
                ++cb_level_;
                item.func();
                --cb_level_;
            }
            run_next_tasks.pop_front();
        }

        while (!run_in_loop_tasks.empty()) {
            RunFuncItem &item = run_in_loop_tasks.front();
            if (item.func) {
                RECORD_SCOPE();
                ++cb_level_;
                item.func();
                --cb_level_;
            }
            run_in_loop_tasks.pop_front();
        }
    }

    if (remain_loop_count == 0)
        LogWarn("found recursive actions, force quit");
}

void CommonLoop::commitRunRequest()
{
    RECORD_SCOPE();
    if (!has_commit_run_req_) {
        uint64_t one = 1;
        ssize_t wsize = write(run_event_fd_, &one, sizeof(one));
        if (wsize != sizeof(one))
            LogErr("write error");

        has_commit_run_req_ = true;
        request_stat_start_ = steady_clock::now();
    }
}

void CommonLoop::finishRunRequest()
{
    auto delay = loop_stat_start_ - request_stat_start_;
    if (delay > water_line_.wake_delay)
        LogNotice("wake_delay: %" PRIu64 " us", delay.count()/1000);

    uint64_t one = 1;
    ssize_t rsize = read(run_event_fd_, &one, sizeof(one));
    if (rsize != sizeof(one))
        LogErr("read error");

    has_commit_run_req_ = false;
}

}
}
