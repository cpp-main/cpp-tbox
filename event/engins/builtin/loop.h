#ifndef TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
#define TBOX_EVENT_LIBEPOLL_LOOP_H_20220105

#include <vector>
#include <functional>

#include "tbox/base/cabinet.hpp"
#include "../../common_loop.h"

#ifndef DEFAULT_MAX_LOOP_ENTRIES
#define DEFAULT_MAX_LOOP_ENTRIES (256)
#endif

namespace tbox {
namespace event {

typedef void(*event_handler)(int fd, uint32_t events, void *obj);

struct EventData
{
    EventData(int f, void *o, event_handler h, uint32_t e)
        : fd(f)
        , obj(o)
        , handler(h)
        , events(e)
    {  }

    int fd;
    void *obj;
    event_handler handler;
    uint32_t events;
};

class BuiltinLoop : public CommonLoop {
  public:
    explicit BuiltinLoop();
    virtual ~BuiltinLoop();

  public:
    virtual void runLoop(Mode mode);
    virtual void exitLoop(const std::chrono::milliseconds &wait_time);

    virtual FdEvent* newFdEvent();
    virtual TimerEvent* newTimerEvent();
    virtual SignalEvent* newSignalEvent();

  public:
    int epollFd() const { return epoll_fd_; }

    using TimerCallback = std::function<void()>;
    cabinet::Token addTimer(uint64_t interval, int64_t repeat, const TimerCallback &cb);
    void deleteTimer(const cabinet::Token &);

  private:
    void onTimeExpired();

  private:
    struct Timer {
        cabinet::Token token;
        uint64_t interval;
        uint64_t expired;
        uint64_t repeat;

        TimerCallback handler;
    };

    struct TimerCmp {
        bool operator()(const Timer *x, const Timer *y) const {
            return x->expired > y->expired;
        }
    };

  private:
    int max_loop_entries_{ DEFAULT_MAX_LOOP_ENTRIES };
    int epoll_fd_{ -1 };
    bool running_{ true };
    TimerEvent *sp_exit_timer_{ nullptr };

    cabinet::Cabinet<Timer> timer_cabinet_;
    std::vector<Timer *> timer_min_heap_;
};

}
}

#endif //TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
