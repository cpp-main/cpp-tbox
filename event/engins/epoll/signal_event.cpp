#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <cassert>
#include <cstdlib>

#include "signal_event.h"

#include <tbox/base/defines.h>
#include <tbox/base/log.h>
#include "loop.h"
#include "fd_event.h"

namespace tbox {
namespace event {

EpollSignalEvent::EpollSignalEvent(EpollLoop *wp_loop) :
    wp_loop_(wp_loop),
    signal_fd_event_(new EpollFdEvent(wp_loop))
{
    sigemptyset(&sig_mask_);
    signal_fd_ = signalfd(-1, &sig_mask_, SFD_NONBLOCK | SFD_CLOEXEC);
    assert(signal_fd_ >= 0);
}

EpollSignalEvent::~EpollSignalEvent()
{
    assert(cb_level_ == 0);
    disable();

    CHECK_DELETE_RESET_OBJ(signal_fd_event_);
    CHECK_CLOSE_RESET_FD(signal_fd_);
}

bool EpollSignalEvent::initialize(int signum, Mode mode)
{
    disable();

    if (!signal_fd_event_->initialize(signal_fd_, FdEvent::kReadEvent, mode))
        return false;

    signal_fd_event_->setCallback(std::bind(&EpollSignalEvent::onEvent, this, std::placeholders::_1));

    sigaddset(&sig_mask_, signum);

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

    signalfd(signal_fd_, &sig_mask_, 0);
    sigprocmask(SIG_BLOCK, &sig_mask_, 0);

    return true;
}

bool EpollSignalEvent::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    sigprocmask(SIG_UNBLOCK, &sig_mask_, 0);
    signalfd(signal_fd_, &sig_mask_, 0);

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
}
