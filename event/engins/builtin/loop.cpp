#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cassert>
#include <errno.h>
#include <string.h>
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

BuiltinLoop::BuiltinLoop()
{
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
}

BuiltinLoop::~BuiltinLoop()
{
    CHECK_CLOSE_RESET_FD(epoll_fd_);
    CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
}

void BuiltinLoop::onTimeExpired()
{
    if (timer_min_heap_.size() <= 0)
        return;

    auto now = CurrentMilliseconds();

    while (timer_min_heap_.size() > 0) {
        auto t = timer_min_heap_.front();
        assert(t != nullptr);

        if (now < t->expired)
            return;

        // The top of timer was expired
        if (t->handler)
            t->handler();

        -- t->repeat;

        // swap first element and last element
        std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
        if (t->repeat > 0) {
            t->expired += t->interval;
            // push the last element to heap again
            std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
        } else {
            // remove the last element
            timer_min_heap_.pop_back();
            CHECK_DELETE_RESET_OBJ(t);
        }
    }
}

void BuiltinLoop::runLoop(Mode mode)
{
    std::vector<struct epoll_event> events;
    events.resize(max_loop_entries_);

    running_ = (mode == Loop::Mode::kForever);
    int64_t wait_time = 0;

    if (epoll_fd_ < 0)
        return;

    runThisBeforeLoop();

    do {
        /// Get the top of minimum heap
        wait_time = timer_min_heap_.size() > 0 ? timer_min_heap_.front()->expired - CurrentMilliseconds() : -1;
        int fds = epoll_wait(epoll_fd_, events.data(), events.size(), wait_time);

        onTimeExpired();

        if (fds <= 0)
            continue;

        for (int i = 0; i < fds; ++i) {
            EventData *event_data = static_cast<EventData *>(events.at(i).data.ptr);
            assert(event_data != nullptr);

            if (event_data->handler)
                event_data->handler(event_data->fd, events.at(i).events, event_data->obj);
        }

        /// If the receiver array size is full, increase its size
        if (fds >= max_loop_entries_) {
            std::vector<struct epoll_event> temp_events;
            temp_events.resize(++max_loop_entries_);
            events.swap(temp_events);
        }

    } while (running_);

    runThisAfterLoop();
}

void BuiltinLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (wait_time.count() == 0) {
        running_ = false;
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
    running_ = false;
}

cabinet::Token BuiltinLoop::addTimer(uint64_t  interval, int64_t repeat, TimerCallback handler)
{
    if (repeat == 0)
        return cabinet::Token();

    if (repeat < 0)
        repeat = std::numeric_limits<int64_t>::max();

    auto now = CurrentMilliseconds();
    Timer *t = new Timer;
    assert(t != nullptr);

    t->expired = now + interval;
    t->interval = interval;
    t->handler = handler;
    t->repeat = repeat;
    t->token = this->timer_cabinet_.insert(t);

    timer_min_heap_.push_back(t);
    std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());

    return t->token;
}

void BuiltinLoop::deleteTimer(const cabinet::Token& token)
{
    std::remove_if(timer_min_heap_.begin(), timer_min_heap_.end(),[token](const Timer *t){
        if (t->token == token) {
            CHECK_DELETE_RESET_OBJ(t);
            return true;
        }
        return false;
    });

    std::make_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
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
