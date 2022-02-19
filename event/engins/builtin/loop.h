#ifndef TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
#define TBOX_EVENT_LIBEPOLL_LOOP_H_20220105

#include <vector>
#include <functional>
#include <unordered_map>

#include "tbox/base/cabinet.hpp"
#include "../../common_loop.h"

#ifndef DEFAULT_MAX_LOOP_ENTRIES
#define DEFAULT_MAX_LOOP_ENTRIES (256)
#endif

namespace tbox {
namespace event {

class EpollFdEventImpl;

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
    inline int epollFd() const { return epoll_fd_; }

    using TimerCallback = std::function<void()>;
    cabinet::Token addTimer(uint64_t interval, uint64_t repeat, const TimerCallback &cb);
    void deleteTimer(const cabinet::Token &token);

    inline void registerFdEvent(int fd, EpollFdEventImpl *fd_event)
    {
        fd_event_map_.insert(std::make_pair(fd, fd_event));
    }

    inline void unregisterFdevent(int fd)
    {
        fd_event_map_.erase(fd);
    }

    inline EpollFdEventImpl *queryFdevent(int fd)
    {
        auto it = fd_event_map_.find(fd);
        if (it != fd_event_map_.end())
            return it->second;

        return nullptr;
    }

  private:
    void onTimeExpired();
    int64_t getWaitTime() const;

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
    int max_loop_entries_{ DEFAULT_MAX_LOOP_ENTRIES };
    int epoll_fd_{ -1 };
    bool keep_running_{ true };
    TimerEvent *sp_exit_timer_{ nullptr };

    cabinet::Cabinet<Timer> timer_cabinet_;
    std::vector<Timer *> timer_min_heap_;

    std::unordered_map<int, EpollFdEventImpl*> fd_event_map_;
};

}
}

#endif //TBOX_EVENT_LIBEPOLL_LOOP_H_20220105
