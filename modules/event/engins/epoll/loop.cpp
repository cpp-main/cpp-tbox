#include <sys/epoll.h>
#include <unistd.h>

#include <cstdint>

#include <algorithm>

#include "loop.h"
#include "timer_event.h"
#include "fd_event.h"

#include <tbox/base/defines.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace event {

namespace {
uint64_t GetCurrentSteadyClockMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds> \
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}

}

EpollLoop::EpollLoop() :
    epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
{
    TBOX_ASSERT(epoll_fd_ >= 0);
}

EpollLoop::~EpollLoop()
{
    cleanupDeferredTasks();

    CHECK_CLOSE_RESET_FD(epoll_fd_);
    CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
}

int64_t EpollLoop::getWaitTime() const
{
    /// Get the top of minimum heap
    int64_t wait_time = -1;
    if (!timer_min_heap_.empty()) {
        wait_time = timer_min_heap_.front()->expired - GetCurrentSteadyClockMilliseconds();
        if (wait_time < 0) //! If expired is little than now, then we consider this timer invalid and trigger it immediately.
            wait_time = 0;
    }

    return wait_time;
}

void EpollLoop::handleExpiredTimers()
{
    auto now = GetCurrentSteadyClockMilliseconds();

    while (!timer_min_heap_.empty()) {
        auto t = timer_min_heap_.front();
        //TBOX_ASSERT(t != nullptr);

        if (now < t->expired)
            break;

        auto tobe_run = t->cb;

        // swap first element and last element
        std::pop_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
        if (UNLIKELY(t->repeat == 1)) {
            // remove the last element
            timer_min_heap_.pop_back();
            timer_cabinet_.free(t->token);
            CHECK_DELETE_RESET_OBJ(t);
        } else {
            t->expired += t->interval;
            // push the last element to heap again
            std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());
            if (LIKELY(t->repeat != 0))
                --t->repeat;
        }

        //! Q: 为什么不在L68执行？
        //! A: 因为要尽可能地将回调放到最后执行。否则不满足测试 TEST(TimerEvent, DisableSelfInCallback)
        if (LIKELY(tobe_run))
            tobe_run();
    }
}

void EpollLoop::runLoop(Mode mode)
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

        handleExpiredTimers();

        if (fds <= 0)
            continue;

        for (int i = 0; i < fds; ++i) {
            epoll_event &ev = events.at(i);
            EpollFdEvent::OnEventCallback(ev.data.fd, ev.events, ev.data.ptr);
        }

        /// If the receiver array size is full, increase its size with 1.5 times.
        if (UNLIKELY(fds >= max_loop_entries_)) {
            max_loop_entries_ = (max_loop_entries_ + max_loop_entries_ / 2);
            events.resize(max_loop_entries_);
        }

    } while (keep_running_);

    runThisAfterLoop();
}

void EpollLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (sp_exit_timer_ != nullptr) {
        sp_exit_timer_->disable();
        CHECK_DELETE_RESET_OBJ(sp_exit_timer_);
    }

    if (wait_time.count() == 0) {
        keep_running_ = false;
    } else {
        sp_exit_timer_ = newTimerEvent();
        sp_exit_timer_->initialize(wait_time, Event::Mode::kOneshot);
        sp_exit_timer_->setCallback([this] { keep_running_ = false; });
        sp_exit_timer_->enable();
    }
}

cabinet::Token EpollLoop::addTimer(uint64_t interval, uint64_t repeat, const TimerCallback &cb)
{
    TBOX_ASSERT(cb);

    auto now = GetCurrentSteadyClockMilliseconds();

    Timer *t = new Timer;
    TBOX_ASSERT(t != nullptr);

    t->token = this->timer_cabinet_.alloc(t);

    t->expired = now + interval;
    t->interval = interval;
    t->cb = cb;
    t->repeat = repeat;

    timer_min_heap_.push_back(t);
    std::push_heap(timer_min_heap_.begin(), timer_min_heap_.end(), TimerCmp());

    return t->token;
}

void EpollLoop::deleteTimer(const cabinet::Token& token)
{
    auto timer = timer_cabinet_.free(token);
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

    run([timer] { delete timer; }); //! Delete later, avoid delete itself
}

void EpollLoop::addFdSharedData(int fd, EpollFdSharedData *fd_event)
{
    fd_data_map_.insert(std::make_pair(fd, fd_event));
}

void EpollLoop::removeFdSharedData(int fd)
{
    fd_data_map_.erase(fd);
}

EpollFdSharedData* EpollLoop::queryFdSharedData(int fd) const
{
    auto it = fd_data_map_.find(fd);
    if (it != fd_data_map_.end())
        return it->second;
    return nullptr;
}


FdEvent* EpollLoop::newFdEvent()
{
    return new EpollFdEvent(this);
}

TimerEvent* EpollLoop::newTimerEvent()
{
    return new EpollTimerEvent(this);
}

}
}
