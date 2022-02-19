#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include <cassert>
#include <cstring>
#include <cstdint>

#include <algorithm>
#include <chrono>
#include <limits>

#include "loop.h"
#include "timer_event.h"
#include "fd_event.h"
#include "signal_event.h"

#include "tbox/base/defines.h"

namespace tbox {
namespace event {

namespace {
uint64_t CurrentMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds> \
        (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

}

BuiltinLoop::BuiltinLoop() :
    epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
{
    assert(epoll_fd_ >= 0);
}

BuiltinLoop::~BuiltinLoop()
{
    CHECK_CLOSE_RESET_FD(epoll_fd_);
    CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
}

int64_t BuiltinLoop::getWaitTime() const
{
    /// Get the top of minimum heap
    int64_t wait_time = -1;
    if (!timer_min_heap_.empty()) {
        wait_time = timer_min_heap_.front()->expired - CurrentMilliseconds();
        if (wait_time < 0) //! If expired is little than now, then we consider this timer invalid and trigger it immediately.
            wait_time = 0;
    }

    return wait_time;
}

void BuiltinLoop::onTimeExpired()
{
    auto now = CurrentMilliseconds();

    while (!timer_min_heap_.empty()) {
        auto t = timer_min_heap_.front();
        assert(t != nullptr);

        if (now < t->expired)
            break;

        // The top of timer was expired
        if (t->cb)
            t->cb();

        // swap first element and last element
        std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
        if (unlikely(t->repeat == 1)) {
            // remove the last element
            timer_min_heap_.pop_back();
            timer_cabinet_.remove(t->token);
            CHECK_DELETE_RESET_OBJ(t);
        } else {
            t->expired += t->interval;
            // push the last element to heap again
            std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
            if (t->repeat != 0)
                --t->repeat;
        }
    }
}

void BuiltinLoop::runLoop(Mode mode)
{
    if (epoll_fd_ < 0)
        return;

    std::vector<struct epoll_event> events;
    /*
     * Why not events.reserve()?
     * The reserve() method only allocates memory, but leaves it uninitialized,
     * it only affects capacity(), but size() will be unchanged.
     * The standard only guarantees that std::vector::data returns a pointer and [data(), data() + size()] is a valid range,
     * the capacity is not concerned. So we need use resize and ensure the [data(), data() + size()] is a valid range whitch used by epoll_wait.
     */
    events.resize(max_loop_entries_);

    runThisBeforeLoop();

    keep_running_ = (mode == Loop::Mode::kForever);
    do {
        int fds = epoll_wait(epoll_fd_, events.data(), events.size(), getWaitTime());

        onTimeExpired();

        if (fds <= 0)
            continue;

        for (int i = 0; i < fds; ++i) {
            epoll_event &ev = events.at(i);
            EpollFdEvent::OnEventCallback(ev.data.fd, ev.events, ev.data.ptr);
        }

        /// If the receiver array size is full, increase its size with 1.5 times.
        if (fds >= max_loop_entries_) {
            max_loop_entries_ = (max_loop_entries_ + max_loop_entries_ / 2);
#if 0
            std::vector<struct epoll_event> temp_events;
            temp_events.resize(max_loop_entries_);
            events.swap(temp_events);
#else
            events.resize(max_loop_entries_);
#endif
        }

    } while (keep_running_);

    runThisAfterLoop();
}

void BuiltinLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (wait_time.count() == 0) {
        keep_running_ = false;
    } else {
        sp_exit_timer_ = newTimerEvent();
        sp_exit_timer_->initialize(wait_time, Event::Mode::kOneshot);
        sp_exit_timer_->setCallback([this] { keep_running_ = false; });
        sp_exit_timer_->enable();
    }
}

cabinet::Token BuiltinLoop::addTimer(uint64_t interval, uint64_t repeat, const TimerCallback &cb)
{
    assert(cb);

    auto now = CurrentMilliseconds();

    Timer *t = new Timer;
    assert(t != nullptr);

    t->token = this->timer_cabinet_.insert(t);

    t->expired = now + interval;
    t->interval = interval;
    t->cb = cb;
    t->repeat = repeat;

    timer_min_heap_.push_back(t);
    std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());

    return t->token;
}

void BuiltinLoop::deleteTimer(const cabinet::Token& token)
{
    auto timer = timer_cabinet_.remove(token);
    if (timer == nullptr)
        return;

#if 0
    timer_min_heap_.erase(timer);
    std::make_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
#else
    //! If we use the above method, it is likely to disrupt order, leading to a wide range of exchanges.
    //! This method will be a little better.
    timer->expired = 0;
    std::make_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
    std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
    timer_min_heap_.pop_back();
#endif

    runNext([timer] { delete timer; }); //! Delete later, avoid delete itself
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
