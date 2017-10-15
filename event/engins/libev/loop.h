#ifndef TBOX_EVENT_LIBEV_LOOP_H_20170826
#define TBOX_EVENT_LIBEV_LOOP_H_20170826

#include "../../common_loop.h"

struct ev_loop;

namespace tbox {
namespace event {

class LibevLoop : public CommonLoop {
  public:
    explicit LibevLoop();
    virtual ~LibevLoop();

  public:
    virtual void runLoop(Mode mode);
    virtual void exitLoop(const std::chrono::milliseconds &wait_time);

    virtual FdItem* newFdItem();
    virtual TimerItem* newTimerItem();
    virtual SignalItem* newSignalItem();

  public:
    struct ev_loop* getEvLoopPtr() const { return sp_ev_loop_; }

  protected:
    void onExitTimeup();

  private:
    struct ev_loop *sp_ev_loop_;
    TimerItem *sp_exit_timer_;
};

}
}

#endif //TBOX_EVENT_LIBEV_LOOP_H_20170826
