#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <deque>
#include <mutex>
#include <thread>

#include "loop.h"

#ifdef ENABLE_STAT
#include <chrono>
#endif

namespace tbox {
namespace event {

class CommonLoop : public Loop {
  public:
    CommonLoop();
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

  protected:
    void runThisBeforeLoop();
    void runThisAfterLoop();

    void onGotRunInLoopFunc(short);

    void cleanupDeferredTasks();

    void commitRequest();
    void finishRequest();

    void handleNextFunc();

  private:
    mutable std::recursive_mutex lock_;

    std::thread::id loop_thread_id_;

    bool has_unhandle_req_ = false;
    int read_fd_ = -1, write_fd_ = -1;
    FdEvent *sp_read_event_ = nullptr;

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

    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
