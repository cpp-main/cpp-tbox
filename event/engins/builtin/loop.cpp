#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <errno.h>
#include <string.h>
#include "loop.h"
#include "timer_event.h"
#include "fd_event.h"
#include "signal_event.h"

namespace tbox {
namespace event {

namespace {
uint64_t CurrentMilliseconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000 + lround(t.tv_nsec / 1e6);
}
}

BuiltinLoop::BuiltinLoop()
{
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
}

BuiltinLoop::~BuiltinLoop()
{
    if (sp_exit_timer_)
        delete sp_exit_timer_;

    close(epoll_fd_);
}

void BuiltinLoop::onTick()
{
    if (timers_.size() <= 0)
        return;

    auto now = CurrentMilliseconds();
    Timer &t = timers_.front();
    if (now < t.expired)
        return;

    // The top of timer was expired
    if (t.handler)
        t.handler();

    -- t.repeat;

    if (t.repeat > 0) {
        t.expired = now + t.interval;
        std::make_heap(timers_.begin(), timers_.end(), cmp_); ///< Just need rebuild the heap
    } else {
        /// Note: pop_heap and pop_back should used by paired.
        std::pop_heap(timers_.begin(), timers_.end(), cmp_);
        timers_.pop_back();
    }
}

void BuiltinLoop::runLoop(Mode mode)
{
    std::array<struct epoll_event, MAX_LOOP_ENTRIES> events;
    memset(events.data(), 0 , events.size());
    valid_ = (mode == Loop::Mode::kForever);
    int64_t wait_time = 0;

    if (epoll_fd_ < 0)
        return;

    runThisBeforeLoop();

    do {
        /// Get the top of minimum heap
        wait_time = timers_.size() > 0 ? timers_.front().expired - CurrentMilliseconds() : -1;
        int fds = epoll_wait(epoll_fd_, events.data(), events.size(), wait_time);

        onTick();

        if (fds <= 0)
            continue;

        for (int i = 0; i < fds; ++i) {
            EventData *event_data = static_cast<EventData *>(events.at(i).data.ptr);
            if (!event_data)
                continue;

            if (event_data->handler)
                event_data->handler(event_data->fd, events.at(i).events, event_data->obj);
        }

    } while (valid_);

    runThisAfterLoop();
}

void BuiltinLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (wait_time.count() == 0) {
        this->valid_ = false;
    } else {
        sp_exit_timer_ = newTimerEvent();
        sp_exit_timer_->initialize(wait_time, Event::Mode::kOneshot);
        sp_exit_timer_->setCallback(std::bind(&BuiltinLoop::onExitTimeup, this));
        sp_exit_timer_->enable();
    }
}

void BuiltinLoop::onExitTimeup()
{
    sp_exit_timer_->disable();
    this->valid_ = false;
}

uint64_t BuiltinLoop::addTimer(uint64_t  interval, int repeat, std::function<void()> handler)
{
    if (repeat == 0)
        return ++timer_id_;

    if (repeat < 0)
        repeat = -1;

    uint64_t now = CurrentMilliseconds();
    Timer t;

    auto timer_id = ++timer_id_;
    t.id = timer_id_;
    t.expired = now + interval;
    t.interval = interval;
    t.handler = handler;
    t.repeat = repeat;

    timers_.push_back(std::move(t));
    std::push_heap(timers_.begin(), timers_.end(), cmp_);

    return timer_id;
}

void BuiltinLoop::delTimer(uint64_t id)
{
    std::remove_if(timers_.begin(), timers_.end(), [id](const Timer &t){ return t.id == id; });
    std::make_heap(timers_.begin(), timers_.end(), cmp_);
}

FdEvent* BuiltinLoop::newFdEvent()
{
    return new EpollFdEvent(this);
}

TimerEvent* BuiltinLoop::newTimerEvent()
{
    return new EpollTimerEvent(this);
}

SignalEvent* BuiltinLoop::newSignalEvent()
{
    return new EpollSignalEvent(this);
}


}
}
