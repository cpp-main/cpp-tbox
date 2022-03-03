#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <deque>
#include <mutex>
#include <thread>
#include <map>
#include <set>

#include "loop.h"
#include "signal_event_impl.h"

#ifdef ENABLE_STAT
#include <chrono>
#endif

namespace tbox {
namespace event {

class CommonLoop : public Loop {
  public:
    ~CommonLoop() override;

  public:
    bool isInLoopThread() override;
    bool isRunning() const override;

    void runInLoop(const Func &func) override;
    void runNext(const Func &func) override;
    void run(const Func &func) override;

    void setStatEnable(bool enable) override;
    bool isStatEnabled() const override;
    Stat getStat() const override;
    void resetStat() override;

  public:
    void beginEventProcess();
    void endEventProcess();

    //! 信号处理相关
    SignalEvent* newSignalEvent() override;
    bool subscribeSignal(int signal_num, SignalSubscribuer *who);
    bool unsubscribeSignal(int signal_num, SignalSubscribuer *who);
    static void HandleSignal(int signo);
    void onSignal();

  protected:
    void runThisBeforeLoop();
    void runThisAfterLoop();

    void handleRunInLoopRequest(short);
    void cleanupDeferredTasks();
    void commitRunRequest();
    void finishRunRequest();
    void handleNextFunc();

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

    static std::map<int, std::set<int>> _signal_write_fds_; //! 通知 Loop 的 fd，每个 Loop 注册一个
    static std::mutex _signal_lock_; //! 保护 _signal_write_fds_ 用

    int signal_read_fd_  = -1;
    int signal_write_fd_ = -1;
    FdEvent *sp_signal_read_event_ = nullptr;
    std::map<int, std::set<SignalSubscribuer*>> all_signals_subscribers_; //! signo -> SignalSubscribuer*，信号的订阅者

    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
