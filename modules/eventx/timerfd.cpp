#include <unistd.h>
#include <sys/timerfd.h>

#include <tbox/event/loop.h>
#include <tbox/base/log.h>
#include <tbox/base/defines.h>

#include "timerfd.h"

namespace tbox {
namespace eventx {

TimerFd::TimerFd(tbox::event::Loop *loop, const std::string &what)
    : tbox::event::TimerEvent(what)
    , loop_(loop)
    , timer_fd_event_(loop->newFdEvent())
{ }

TimerFd::~TimerFd()
{
    timer_fd_event_->disable();
    CHECK_CLOSE_FD(timer_fd_);
    CHECK_DELETE_OBJ(timer_fd_event_);
}

bool TimerFd::isEnabled() const
{
    return is_enabled_;
}

bool TimerFd::enable()
{
    if (!is_inited_)
        return false;

    if (is_enabled_)
        return true;

    is_enabled_ = timer_fd_event_->enable();
    return is_enabled_;
}

bool TimerFd::disable()
{
    if (!is_inited_)
        return false;

    if (!is_enabled_)
        return true;

    if (timer_fd_event_->disable()) {
        is_enabled_ = false;
        return true;
    }

    return false;
}

tbox::event::Loop* TimerFd::getLoop() const
{
    return loop_;
}

bool TimerFd::initialize(const std::chrono::milliseconds &time_span, Mode mode)
{
    std::chrono::microseconds mics = std::chrono::duration_cast<std::chrono::microseconds>(time_span);
    return initialize(mics, mode);
}

bool TimerFd::initialize(const std::chrono::microseconds &time_span, Mode mode)
{
    std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_span);
    return initialize(ns, mode);
}

bool TimerFd::initialize(const std::chrono::nanoseconds &time_span, Mode mode)
{
    CHECK_CLOSE_FD(timer_fd_);
    timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd_ < 0)
        return false;

    struct itimerspec ts;

    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(time_span);
    std::chrono::nanoseconds n = time_span - s;

    ts.it_interval.tv_sec = s.count();
    ts.it_interval.tv_nsec = n.count();
    ts.it_value.tv_sec = s.count();
    ts.it_value.tv_nsec = n.count();

    if (timerfd_settime(timer_fd_, TFD_TIMER_CANCEL_ON_SET, &ts, NULL) < 0) {
        LogWarn("timerfd_settime() failed: errno=%d", errno);
        return false;
    }

    timer_fd_event_->initialize(timer_fd_, tbox::event::FdEvent::kReadEvent, mode);
    timer_fd_event_->setCallback(std::bind(&TimerFd::onEvent, this, std::placeholders::_1));
    is_inited_ = true;
    return is_inited_;
}

void TimerFd::onEvent(short events)
{
    if (events & tbox::event::FdEvent::kReadEvent) {
        uint64_t exp = 0;
        int len  = read(timer_fd_, &exp, sizeof(exp));
        if (len <= 0)
            return;

        if (cb_)
            cb_();
    }
}

void TimerFd::setCallback(CallbackFunc &&cb)
{
    cb_ = cb;
}

}
}
