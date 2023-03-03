#include "common_loop.h"

#include <unistd.h>
#include <string.h>
#include <tbox/base/defines.h>
#include <tbox/base/log.h>

#include "misc.h"
#include "fd_event.h"

namespace tbox {
namespace event {

namespace {

using SignalHandler = void (*) (int);

struct SignalCtx {
    std::set<int> write_fds;    //! 通知 Loop 的 fd，每个 Loop 注册一个
    SignalHandler old_handler;
};

std::mutex _signal_lock_;    //! 保护 _signal_ctxs_ 用
std::map<int, SignalCtx> _signal_ctxs_;

//! 信号处理函数
void SignalHandlerFunc(int signo)
{
    /// 注意: 这里不能使用 LogXXX() 打印日志，因为有死锁的风险
    ///       信号处理函数中也不应该有锁相关的操作

    const auto &signal_ctx = _signal_ctxs_[signo];

    //! 先执行旧的信号
    auto old_handler = signal_ctx.old_handler;
    if (SIG_ERR != old_handler &&
        SIG_IGN != old_handler &&
        SIG_DFL != old_handler)
        old_handler(signo);

    //! 再执行自己的
    for (int fd : signal_ctx.write_fds) {
        auto wsize = write(fd, &signo, sizeof(signo));
        (void)wsize;    //! 消除编译警告
    }
}

}

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
        //! 如果本Loop没有监听该信号，则要去 _signal_ctxs_ 中订阅
        std::unique_lock<std::mutex> _g(_signal_lock_);

        //! 要禁止信号触发
        sigset_t new_sigmask, old_sigmask;
        sigfillset(&new_sigmask);
        sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);

        auto & signal_ctx = _signal_ctxs_[signo];
        if (signal_ctx.write_fds.empty()) {
            signal_ctx.old_handler = ::signal(signo, SignalHandlerFunc);
            if (SIG_ERR == signal_ctx.old_handler)
                LogWarn("install signal %d fail", signo);
        }
        signal_ctx.write_fds.insert(signal_write_fd_);

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

        //! 并将 _signal_ctxs_ 中的记录删除
        auto &signal_ctx = _signal_ctxs_[signo];
        signal_ctx.write_fds.erase(signal_write_fd_);
        if (signal_ctx.write_fds.empty()) {
            //! 并还原信号处理函数
            if (signal_ctx.old_handler != SIG_ERR)
                ::signal(signo, signal_ctx.old_handler);
            //LogTrace("unset signal:%d", signo);
            _signal_ctxs_.erase(signo);
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
