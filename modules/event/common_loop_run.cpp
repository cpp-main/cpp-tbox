#include "common_loop.h"

#include <unistd.h>
#include <inttypes.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace event {

using namespace std::chrono;

CommonLoop::RunFuncItem::RunFuncItem(const Func &f, const std::string &w)
    : commit_time_point(steady_clock::now())
    , func(f)
    , what(w)
{ }

void CommonLoop::runInLoop(const Func &func, const std::string &what)
{
    std::lock_guard<std::recursive_mutex> g(lock_);

    run_in_loop_func_queue_.emplace_back(RunFuncItem(func, what));

    auto queue_size = run_in_loop_func_queue_.size();
    if (queue_size > run_in_loop_queue_size_water_line_)
        LogWarn("run_in_loop_queue size: %u", queue_size);

    if (queue_size > run_in_loop_peak_num_)
        run_in_loop_peak_num_ = queue_size;

    if (sp_run_read_event_ != nullptr)
        commitRunRequest();
}

void CommonLoop::runNext(const Func &func, const std::string &what)
{
#ifdef DEBUG
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        //! 要么还不有启动，要么在Loop线程中才允行这个操作
        TBOX_ASSERT(!isRunningLockless() || isInLoopThreadLockless());
    }
#endif
    run_next_func_queue_.emplace_back(RunFuncItem(func, what));

    auto queue_size = run_next_func_queue_.size();
    if (queue_size > run_next_queue_size_water_line_)
        LogWarn("run_next_queue size: %u", queue_size);

    if (queue_size > run_next_peak_num_)
        run_next_peak_num_ = queue_size;
}

void CommonLoop::run(const Func &func, const std::string &what)
{
    bool can_run_next = true;
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        if (isRunningLockless() && !isInLoopThreadLockless())
            can_run_next = false;
    }

    if (can_run_next)
        runNext(func, what);
    else
        runInLoop(func, what);
}

void CommonLoop::handleNextFunc()
{
    std::deque<RunFuncItem> tmp;
    run_next_func_queue_.swap(tmp);

    while (!tmp.empty()) {
        RunFuncItem &item = tmp.front();

        auto now = steady_clock::now();
        auto delay = now - item.commit_time_point;
        if (delay > run_next_delay_waterline_)
            LogWarn("run next delay over waterline: %" PRIu64 " us, what: '%s'",
                    delay.count()/1000, item.what.c_str());

        if (item.func) {
            ++cb_level_;
            item.func();
            --cb_level_;
        }

        auto cost = steady_clock::now() - now;
        if (cost > cb_time_cost_waterline_)
            LogWarn("run next cost over waterline: %" PRIu64 " us, what: '%s'",
                    cost.count()/1000, item.what.c_str());

        tmp.pop_front();
    }
}

bool CommonLoop::hasNextFunc() const
{
    return !run_next_func_queue_.empty();
}

void CommonLoop::handleRunInLoopFunc()
{
    /**
     * NOTICE:
     * 这里使用 tmp 将 run_in_loop_func_queue_ 中的内容交换出去。然后再从 tmp 逐一取任务出来执行。
     * 其目的在于腾空 run_in_loop_func_queue_，让新 runInLoop() 的任务则会在下一轮循环中执行。
     * 从而防止无限 runInLoop() 引起的死循环，导致其它事件得不到处理。
     *
     * 这点与 runNext() 不同
     */
    std::deque<RunFuncItem> tmp;
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        run_in_loop_func_queue_.swap(tmp);
        finishRunRequest();
    }

    while (!tmp.empty()) {
        RunFuncItem &item = tmp.front();

        auto now = steady_clock::now();
        auto delay = now - item.commit_time_point;
        if (delay > run_in_loop_delay_waterline_)
            LogWarn("run in loop delay over waterline: %" PRIu64 " us, what: '%s'",
                    delay.count()/1000, item.what.c_str());

        if (item.func) {
            ++cb_level_;
            item.func();
            --cb_level_;
        }

        auto cost = steady_clock::now() - now;
        if (cost > cb_time_cost_waterline_)
            LogWarn("run next cost over waterline: %" PRIu64 " us, what: '%s'",
                    cost.count()/1000, item.what.c_str());

        tmp.pop_front();
    }
}

//! 清理 run_in_loop_func_queue_ 与 run_next_func_queue_ 中的任务
void CommonLoop::cleanupDeferredTasks()
{
    int remain_loop_count = 10; //! 限定次数，防止出现 runNext() 递归导致无法退出循环的问题
    while ((!run_in_loop_func_queue_.empty() || !run_next_func_queue_.empty()) && remain_loop_count-- > 0) {

        std::deque<RunFuncItem> run_next_tasks = std::move(run_next_func_queue_);
        std::deque<RunFuncItem> run_in_loop_tasks = std::move(run_in_loop_func_queue_);

        while (!run_next_tasks.empty()) {
            RunFuncItem &item = run_next_tasks.front();
            if (item.func) {
                ++cb_level_;
                item.func();
                --cb_level_;
            }
            run_next_tasks.pop_front();
        }

        while (!run_in_loop_tasks.empty()) {
            RunFuncItem &item = run_in_loop_tasks.front();
            if (item.func) {
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
    if (!has_commit_run_req_) {
        char ch = 0;
        ssize_t wsize = write(run_write_fd_, &ch, 1);
        (void)wsize;

        has_commit_run_req_ = true;
        request_stat_start_ = steady_clock::now();
    }
}

void CommonLoop::finishRunRequest()
{
    auto delay = steady_clock::now() - request_stat_start_;
    if (delay > run_request_delay_waterline_)
        LogWarn("run request delay over waterline: %" PRIu64 " us", delay.count()/1000);

    char ch = 0;
    ssize_t rsize = read(run_read_fd_, &ch, 1);
    (void)rsize;

    has_commit_run_req_ = false;
}

}
}
