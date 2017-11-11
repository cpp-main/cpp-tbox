#ifndef TBOX_EVENT_COMMON_LOOP_H_20170713
#define TBOX_EVENT_COMMON_LOOP_H_20170713

#include <deque>
#include <mutex>
#include <thread>

#include "loop.h"

#ifdef ENABLE_STAT
#include <chrono>
using std::chrono::steady_clock;
#endif

namespace tbox {
namespace event {

class CommonLoop : public Loop {
  public:
    CommonLoop();
    virtual ~CommonLoop();

  public:
    virtual bool isInLoopThread();
    virtual void runInLoop(const RunInLoopFunc &func);

    virtual Stat getStat() const;
    virtual void resetStat();

  public:
#ifdef ENABLE_STAT
    void recordTimeCost(uint64_t cost_us);
#endif  //ENABLE_STAT

  protected:
    void runThisBeforeLoop();
    void runThisAfterLoop();

    void onGotRunInLoopFunc(short);

    void commitRequest();
    void finishRequest();

  private:
    std::mutex lock_;

    std::thread::id loop_thread_id_;

    bool has_unhandle_req_;
    int read_fd_, write_fd_;
    FdEvent *sp_read_event_;
    std::deque<RunInLoopFunc> func_queue_;

#ifdef ENABLE_STAT
    steady_clock::time_point stat_start_;
    uint64_t time_cost_us_ = 0;
    uint32_t event_count_ = 0;
    uint32_t max_cost_us_ = 0;
#endif  //ENABLE_STAT

    int cb_level_;
};

}
}

#endif //TBOX_EVENT_COMMON_LOOP_H_20170713
