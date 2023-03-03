#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <deque>
#include <mutex>
#include <thread>
#include <map>
#include <set>

#include <tbox/base/cabinet.hpp>

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

    virtual void runInLoop(const Func &func) override;
    virtual void runNext(const Func &func) override;
    virtual void run(const Func &func) override;

    virtual void setStatEnable(bool enable) override;
    virtual bool isStatEnabled() const override;
    virtual Stat getStat() const override;
    virtual void resetStat() override;

    virtual void exitLoop(const std::chrono::milliseconds &wait_time) override;

  public:
    void beginEventProcess();
    void endEventProcess();

    //! 信号处理相关
    virtual SignalEvent* newSignalEvent() override;
    bool subscribeSignal(int signal_num, SignalSubscribuer *who);
    bool unsubscribeSignal(int signal_num, SignalSubscribuer *who);
    void onSignal();

    virtual TimerEvent* newTimerEvent() override;
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

  private:
    mutable std::recursive_mutex lock_;
    std::thread::id loop_thread_id_;
    bool has_commit_run_req_ = false;
    int run_read_fd_ = -1;
    int run_write_fd_ = -1;
    FdEvent *sp_run_read_event_ = nullptr;
    std::deque<Func> run_in_loop_func_queue_;
    std::deque<Func> run_next_func_queue_;

#ifdef ENABLE_STAT
    bool stat_enable_ = false;
    std::chrono::steady_clock::time_point whole_stat_start_;
    std::chrono::steady_clock::time_point event_stat_start_;
    uint64_t time_cost_us_ = 0;
    uint32_t event_count_ = 0;
    uint32_t max_cost_us_ = 0;
#endif  //ENABLE_STAT

    int signal_read_fd_  = -1;
    int signal_write_fd_ = -1;
    FdEvent *sp_signal_read_event_ = nullptr;
    std::map<int, std::set<SignalSubscribuer*>> all_signals_subscribers_; //! signo -> SignalSubscribuer*，信号的订阅者

    TimerEvent *sp_exit_timer_ = nullptr;
    cabinet::Cabinet<Timer> timer_cabinet_;
    std::vector<Timer*>     timer_min_heap_;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
