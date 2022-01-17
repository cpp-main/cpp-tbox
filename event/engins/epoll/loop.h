#ifndef TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
#define TBOX_EVENT_LIBEPOLL_LOOP_H_20220105

#include <vector>
#include <functional>
#include "../../common_loop.h"

#ifndef MAX_LOOP_ENTRIES
#define MAX_LOOP_ENTRIES (1024)
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

class EpollLoop : public CommonLoop {
    struct Timer
    {
        uint64_t interval;
        uint64_t expired;
        uint64_t repeat;
        uint64_t id;

        std::function<void()> handler;
    };

  public:
    explicit EpollLoop();
    virtual ~EpollLoop();

  public:
    virtual void runLoop(Mode mode);
    virtual void exitLoop(const std::chrono::milliseconds &wait_time);

    virtual FdEvent* newFdEvent();
    virtual TimerEvent* newTimerEvent();
    virtual SignalEvent* newSignalEvent();

  public:
    int epollFd() { return epoll_fd_; }
    uint64_t addTimer(uint64_t interval, int repeat, std::function<void()> handler = nullptr);
    void delTimer(uint64_t id);

  protected:
    void onExitTimeup();

  private:
    void onTick();

  private:
    int epoll_fd_{ -1 };
    bool valid_{ true };
    TimerEvent *sp_exit_timer_{ nullptr };

    std::vector<Timer> timers_{ };
    uint64_t timer_id_{ 0 };
    std::function<bool (const Timer &x, const Timer &y)> cmp_{[](const Timer &x, const Timer &y){ return x.expired > y. expired; }};
};

}
}

#endif //TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
