#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <cassert>
#include <cstdlib>

#include "signal_event.h"

#include <tbox/base/defines.h>
#include "loop.h"
#include "fd_event.h"

namespace tbox::event {

EpollSignalEvent::EpollSignalEvent(BuiltinLoop *wp_loop) :
     wp_loop_(wp_loop)
    ,signal_fd_event_(new EpollFdEvent(wp_loop))
{ }

EpollSignalEvent::~EpollSignalEvent()
{
    assert(cb_level_ == 0);
    disable();

    CHECK_DELETE_RESET_OBJ(signal_fd_event_);
}

bool EpollSignalEvent::initialize(int signum, Mode mode)
{
    disable();

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signum);
    if (sigprocmask(SIG_BLOCK, &mask, 0) == -1)
        return false;

    signal_fd_ = signalfd(-1, &mask, 0);
    if (signal_fd_ == -1)
        return false;

    if (!signal_fd_event_->initialize(signal_fd_, FdEvent::kReadEvent, mode))
        return false;

    signal_fd_event_->setCallback(std::bind(&EpollSignalEvent::onEvent, this, std::placeholders::_1));

    if (mode == Mode::kOneshot)
        is_stop_after_trigger_ = true;

    is_inited_ = true;
    return true;
}

void EpollSignalEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool EpollSignalEvent::isEnabled() const
{
    if (!is_inited_)
        return false;

    return signal_fd_event_->isEnabled();
}

bool EpollSignalEvent::enable()
{
    if (!is_inited_)
        return false;

    if (isEnabled())
        return true;

    if (!signal_fd_event_->enable())
        return false;

    return true;
}

bool EpollSignalEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    if (!signal_fd_event_->disable())
        return false;

    return true;
}

Loop* EpollSignalEvent::getLoop() const
{
    return wp_loop_;
}

void EpollSignalEvent::onEvent(short events)
{
    wp_loop_->beginEventProcess();

    if (!(events & FdEvent::kReadEvent))
        return;

    /// We need read the signal_fd_ if got some signals
    struct signalfd_siginfo info;
    if (read(signal_fd_, &info, sizeof(info)) != sizeof(info))
        return;

    /// signal number = info.ssi_signo
    /// pid = info.ssi_pid
    /// uid = info.ssi_uid

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();
    }

    wp_loop_->endEventProcess();
}

}
