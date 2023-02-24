#include "common_loop.h"

#include <unistd.h>
#include <string.h>
#include <tbox/base/defines.h>
#include <tbox/base/log.h>

#include "misc.h"
#include "fd_event.h"

namespace tbox {
namespace event {

std::map<int, std::set<int>> CommonLoop::_signal_write_fds_;
std::mutex    CommonLoop::_signal_lock_;

bool CommonLoop::subscribeSignal(int signo, SignalSubscribuer *who)
{
    if (signal_read_fd_ == -1) {    //! 如果还没有创建对应的信号
        if (!CreateFdPair(signal_read_fd_, signal_write_fd_))
            return false;

        sp_signal_read_event_ = newFdEvent();
        sp_signal_read_event_->initialize(signal_read_fd_, FdEvent::kReadEvent, Event::Mode::kPersist);
        sp_signal_read_event_->setCallback(std::bind(&CommonLoop::onSignal, this));
        sp_signal_read_event_->enable();
    }

    auto &this_signal_subscribers = all_signals_subscribers_[signo];
    if (this_signal_subscribers.empty()) {
        //! 如果本Loop没有监听该信号，则要去 _signal_write_fds_ 中订阅
        std::unique_lock<std::mutex> _g(_signal_lock_);

        //! 要禁止信号触发
        sigset_t new_sigmask, old_sigmask;
        sigfillset(&new_sigmask);
        sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);

        auto & signo_fds = _signal_write_fds_[signo];
        if (signo_fds.empty()) {
            signal(signo, CommonLoop::OnSignal);
            //LogTrace("set signal:%d", signo);
        }
        signo_fds.insert(signal_write_fd_);

        //! 恢复信号
        sigprocmask(SIG_SETMASK, &old_sigmask, 0);
    }
    this_signal_subscribers.insert(who);

    return true;
}

bool CommonLoop::unsubscribeSignal(int signo, SignalSubscribuer *who)
{
    auto &this_signal_subscribers = all_signals_subscribers_[signo];
    this_signal_subscribers.erase(who);          //! 将订阅信息删除
    if (!this_signal_subscribers.empty())        //! 检查本Loop中是否已经没有SignalSubscribuer订阅该信号了
        return true;    //! 如果还有，就到此为止

    //! 如果本Loop已经没有SignalSubscribuer订阅该信号了
    all_signals_subscribers_.erase(signo);    //! 则将该信号的订阅记录表删除
    {
        std::unique_lock<std::mutex> _g(_signal_lock_);

        //! 要禁止信号触发
        sigset_t new_sigmask, old_sigmask;
        sigfillset(&new_sigmask);
        sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);

        //! 并将 _signal_write_fds_ 中的记录删除
        auto &this_signal_fds = _signal_write_fds_[signo];
        this_signal_fds.erase(signal_write_fd_);
        if (this_signal_fds.empty()) {
            //! 并还原信号处理函数
            signal(signo, SIG_DFL);
            //LogTrace("unset signal:%d", signo);
            _signal_write_fds_.erase(signo);
        }

        //! 恢复信号
        sigprocmask(SIG_SETMASK, &old_sigmask, 0);
    }

    if (!all_signals_subscribers_.empty())
        return true;

    //! 已经没有任何SignalSubscribuer订阅任何信号了
    sp_signal_read_event_->disable();
    CHECK_CLOSE_RESET_FD(signal_write_fd_);
    CHECK_CLOSE_RESET_FD(signal_read_fd_);

    FdEvent *tobe_delete = nullptr;
    std::swap(tobe_delete, sp_signal_read_event_);
    run([tobe_delete] { delete tobe_delete; });

    return true;
}

//! 信号处理函数
void CommonLoop::OnSignal(int signo)
{
    auto &this_signal_fds = _signal_write_fds_[signo];
    for (int fd : this_signal_fds) {
        auto wsize = write(fd, &signo, sizeof(signo));
        (void)wsize;    //! 消除编译警告
    }
}

void CommonLoop::onSignal()
{
    while (signal_read_fd_ != -1) {
        int signo_array[10];    //! 一次性读10个
        auto rsize = read(signal_read_fd_, &signo_array, sizeof(signo_array));
        if (rsize > 0) {
            const auto num = rsize / sizeof(int);
            //LogTrace("rsize:%d, num:%u", rsize, num);
            for (size_t i = 0; i < num; ++i) {
                int signo = signo_array[i];
                //LogTrace("signo:%d", signo);
                auto iter = all_signals_subscribers_.find(signo);
                if (iter != all_signals_subscribers_.end()) {
                    auto todo = iter->second;   //!FIXME:Crash if SignalSubscribuer be deleted in callback
                    for (auto s : todo) {
                        s->onSignal(signo);
                    }
                }
            }
        } else {
            if (errno != EAGAIN)
                LogWarn("read error, rsize:%d, errno:%d, %s", rsize, errno, strerror(errno));
            break;
        }
    }
}

SignalEvent* CommonLoop::newSignalEvent()
{
    return new SignalEventImpl(this);
}

}
}
