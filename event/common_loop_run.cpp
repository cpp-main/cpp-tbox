#include "common_loop.h"

#include <unistd.h>
#include <tbox/base/log.h>

namespace tbox {
namespace event {

void CommonLoop::runInLoop(const Func &func)
{
    std::lock_guard<std::recursive_mutex> g(lock_);
    run_in_loop_func_queue_.push_back(func);

    if (sp_run_read_event_ == nullptr)
        return;

    commitRunRequest();
}

void CommonLoop::runNext(const Func &func)
{
    if (!isInLoopThread()) {
        LogWarn("Fail, use runInLoop() instead.");
        return;
    }

    run_next_func_queue_.push_back(func);
}

void CommonLoop::run(const Func &func)
{
    if (isInLoopThread())
        run_next_func_queue_.push_back(func);
    else
        runInLoop(func);
}

void CommonLoop::handleNextFunc()
{
    while (!run_next_func_queue_.empty()) {
        Func &func = run_next_func_queue_.front();
        if (func) {
            ++cb_level_;
            func();
            --cb_level_;
        }
        run_next_func_queue_.pop_front();
    }
}

void CommonLoop::handleRunInLoopRequest(short)
{
    /**
     * NOTICE:
     * 这里使用 tmp 将 run_in_loop_func_queue_ 中的内容交换出去。然后再从 tmp 逐一取任务出来执行。
     * 其目的在于腾空 run_in_loop_func_queue_，让新 runInLoop() 的任务则会在下一轮循环中执行。
     * 从而防止无限 runInLoop() 引起的死循环，导致其它事件得不到处理。
     *
     * 这点与 runNext() 不同
     */
    std::deque<Func> tmp;
    {
        std::lock_guard<std::recursive_mutex> g(lock_);
        run_in_loop_func_queue_.swap(tmp);
        finishRunRequest();
    }

    while (!tmp.empty()) {
        Func &func = tmp.front();
        if (func) {
            ++cb_level_;
            func();
            --cb_level_;

            handleNextFunc();
        }
        tmp.pop_front();
    }
}

//! 清理 run_in_loop_func_queue_ 与 run_next_func_queue_ 中的任务
void CommonLoop::cleanupDeferredTasks()
{
    int remain_loop_count = 10; //! 限定次数，防止出现 runNext() 递归导致无法退出循环的问题
    while ((!run_in_loop_func_queue_.empty() || !run_next_func_queue_.empty())
            && remain_loop_count-- > 0) {
        std::deque<Func> tasks = std::move(run_next_func_queue_);
        tasks.insert(tasks.end(), run_in_loop_func_queue_.begin(), run_in_loop_func_queue_.end());
        run_in_loop_func_queue_.clear();

        while (!tasks.empty()) {
            Func &func = tasks.front();
            if (func) {
                ++cb_level_;
                func();
                --cb_level_;
            }
            tasks.pop_front();
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
    }
}

void CommonLoop::finishRunRequest()
{
    char ch = 0;
    ssize_t rsize = read(run_read_fd_, &ch, 1);
    (void)rsize;

    has_commit_run_req_ = false;
}

}
}
