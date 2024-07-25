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
#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <deque>
#include <mutex>
#include <thread>
#include <map>
#include <set>

#include <tbox/base/cabinet.hpp>
#include <tbox/base/object_pool.hpp>

#include "loop.h"
#include "signal_event_impl.h"

#include <chrono>

namespace tbox {
namespace event {

class CommonLoop : public Loop {
  public:
    virtual ~CommonLoop() override;

  public:
    virtual bool isInLoopThread() override;
    virtual bool isRunning() const override;

    virtual RunId runInLoop(Func &&func, const std::string &what) override;
    virtual RunId runInLoop(const Func &func, const std::string &what) override;
    virtual RunId runNext(Func &&func, const std::string &what) override;
    virtual RunId runNext(const Func &func, const std::string &what) override;
    virtual RunId run(Func &&func, const std::string &what) override;
    virtual RunId run(const Func &func, const std::string &what) override;
    virtual bool  cancel(RunId run_id) override;

    virtual Stat getStat() const override;
    virtual void resetStat() override;

    virtual void exitLoop(const std::chrono::milliseconds &wait_time) override;

    WaterLine& water_line() override { return water_line_; }

    virtual void cleanup() override;

  public:
    void beginLoopProcess();
    void endLoopProcess();

    void beginEventProcess();
    void endEventProcess(Event *event);

    //! Signal 相关
    virtual SignalEvent* newSignalEvent(const std::string &what) override;
    bool subscribeSignal(int signal_num, SignalSubscribuer *who);
    bool unsubscribeSignal(int signal_num, SignalSubscribuer *who);
    void onSignal();

    //! Timer 相关
    virtual TimerEvent* newTimerEvent(const std::string &what) override;
    using TimerCallback = std::function<void()>;
    cabinet::Token addTimer(uint64_t interval, uint64_t repeat, const TimerCallback &cb);
    void deleteTimer(const cabinet::Token &token);

  protected:
    bool isInLoopThreadLockless() const;
    bool isRunningLockless() const;

    void runThisBeforeLoop();
    void runThisAfterLoop();

    void handleRunInLoopRequest(short);
    void cleanupDeferredTasks();

    void commitRunRequest();
    void finishRunRequest();
    void handleRunInLoopFunc();
    void handleNextFunc();
    bool hasNextFunc() const;

    void handleExpiredTimers();
    int64_t getWaitTime() const;

    virtual void stopLoop() = 0;

  private:
    struct Timer {
        cabinet::Token token;
        uint64_t interval = 0;
        uint64_t expired = 0;
        uint64_t repeat = 0;

        TimerCallback cb;
    };

    struct TimerCmp {
        bool operator()(const Timer *x, const Timer *y) const {
            return x->expired > y->expired;
        }
    };

    struct RunFuncItem {
        RunFuncItem(RunId id, Func &&func, const std::string &what);

        RunId id;
        std::chrono::steady_clock::time_point commit_time_point;
        Func func;
        std::string what;
    };

    using RunFuncQueue = std::deque<RunFuncItem>;

    RunId allocRunInLoopId();
    RunId allocRunNextId();

    static bool RemoveRunFuncItemById(RunFuncQueue &run_deqeue, RunId run_id);

  private:
    mutable std::recursive_mutex lock_;
    std::thread::id loop_thread_id_;
    int cb_level_ = 0;

    //! run 相关
    bool has_commit_run_req_ = false;
    int run_event_fd_ = -1;
    FdEvent *sp_run_read_event_ = nullptr;
    RunId run_in_loop_id_alloc_ = 0;    //! 偶数
    RunId run_next_id_alloc_ = 1;       //! 奇数
    RunFuncQueue run_in_loop_func_queue_;
    RunFuncQueue run_next_func_queue_;
    RunFuncQueue tmp_func_queue_;   //! 当前将要立即执行的任务队列

    //! 统计相关
    std::chrono::steady_clock::time_point whole_stat_start_;
    std::chrono::steady_clock::time_point loop_stat_start_;
    uint32_t loop_count_ = 0;         //!< loop次数
    std::chrono::nanoseconds loop_acc_cost_;   //!< loop工作累积时长
    std::chrono::nanoseconds loop_peak_cost_;  //!< loop工作最长时长

    size_t run_in_loop_peak_num_ = 0; //!< 等待任务数峰值
    size_t run_next_peak_num_ = 0;    //!< 等待任务数峰值

    //! Signal 相关
    int signal_read_fd_  = -1;
    int signal_write_fd_ = -1;
    FdEvent *sp_signal_read_event_ = nullptr;
    std::map<int, std::set<SignalSubscribuer*>> all_signals_subscribers_; //! signo -> SignalSubscribuer*，信号的订阅者

    //! Timer 相关
    TimerEvent *sp_exit_timer_ = nullptr;
    cabinet::Cabinet<Timer> timer_cabinet_;
    std::vector<Timer*>     timer_min_heap_;
    ObjectPool<Timer>       timer_object_pool_{64};

    //! 警告水位线
    WaterLine water_line_ = {
      .run_in_loop_queue_size = std::numeric_limits<size_t>::max(),
      .run_next_queue_size = std::numeric_limits<size_t>::max(),
      .wake_delay = std::chrono::milliseconds(5),
      .loop_cost = std::chrono::milliseconds(100),
      .event_cb_cost = std::chrono::milliseconds(50),
      .run_cb_cost = std::chrono::milliseconds(50),
      .run_in_loop_delay = std::chrono::milliseconds(10),
      .run_next_delay = std::chrono::milliseconds(10),
      .timer_delay = std::chrono::milliseconds(10)
    };

    std::chrono::steady_clock::time_point event_cb_stat_start_;
    std::chrono::steady_clock::time_point request_stat_start_;

};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
