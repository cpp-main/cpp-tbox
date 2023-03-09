#include <signal.h>
#include <execinfo.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <functional>
#include <map>

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/util/backtrace.h>

#define TBOX_USE_SIGACTION

int main(int argc, char **argv);

namespace tbox {
namespace main {

extern std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

namespace {

#ifdef  TBOX_USE_SIGACTION
using SignalHandler = struct sigaction;
#else
using SignalHandler = void (*) (int);
#endif

std::map<int, SignalHandler> _old_handler_map;

#ifdef  TBOX_USE_SIGACTION
void InvokeOldHandler(int signo, siginfo_t *siginfo, void *context)
#else
void InvokeOldHandler(int signo)
#endif
{
    auto iter = _old_handler_map.find(signo);
    if (iter == _old_handler_map.end())
        return;

    const auto &old_handler = iter->second;

#ifdef  TBOX_USE_SIGACTION
    if (old_handler.sa_flags & SA_SIGINFO) {
        if (old_handler.sa_sigaction)
            old_handler.sa_sigaction(signo, siginfo, context);
    } else {
        if (SIG_ERR != old_handler.sa_handler &&
            SIG_IGN != old_handler.sa_handler &&
            SIG_DFL != old_handler.sa_handler)
            old_handler.sa_handler(signo);
    }
#else
    if (SIG_ERR != old_handler &&
        SIG_IGN != old_handler &&
        SIG_DFL != old_handler)
        old_handler(signo);
#endif
}

//! 处理程序运行异常信号
#ifdef TBOX_USE_SIGACTION
void OnErrorSignal(int signo, siginfo_t *siginfo, void *context)
#else
void OnErrorSignal(int signo)
#endif
{
#ifdef TBOX_USE_SIGACTION
    InvokeOldHandler(signo, siginfo, context);
#else
    InvokeOldHandler(signo);
#endif

    const std::string &stack_str = util::DumpCallStack(32);

    LogFatal("Receive signal %d", signo);
    LogFatal("main: <%p>\n-----call stack-----\n%s",
      ::main, stack_str.c_str());

    if (error_exit_func)    //! 执行一些善后处理
        error_exit_func();

    signal(SIGABRT, SIG_DFL);
    std::abort();
}

#ifdef TBOX_USE_SIGACTION
void OnWarnSignal(int signo, siginfo_t *siginfo, void *context)
#else
void OnWarnSignal(int signo)
#endif
{
#ifdef TBOX_USE_SIGACTION
    InvokeOldHandler(signo, siginfo, context);
#else
    InvokeOldHandler(signo);
#endif

    LogWarn("Receive signal %d", signo);
}

bool InstallSignal(int signo, bool as_error)
{
    SignalHandler old_handler;
    bool is_succ = false;
    auto func = as_error ? OnErrorSignal : OnWarnSignal;

#ifdef TBOX_USE_SIGACTION
    struct sigaction new_handler;
    memset(&new_handler, 0, sizeof(new_handler));
    sigemptyset(&new_handler.sa_mask);
    new_handler.sa_sigaction = func;
    new_handler.sa_flags = SA_SIGINFO;
    ssize_t ret = ::sigaction(signo, &new_handler, &old_handler);
    is_succ = (ret != -1);
#else
    old_handler = ::signal(signo, func);
    is_succ = (SIG_ERR != old_handler);
#endif

    _old_handler_map[signo] = old_handler;
    return is_succ;
}

bool UninstallSignal(int signo)
{
    auto iter = _old_handler_map.find(signo);
    if (iter == _old_handler_map.end())
        return false;

    auto &old_handler = iter->second;

#ifdef TBOX_USE_SIGACTION
    ::sigaction(signo, &old_handler, nullptr);
#else
    if (old_handler != SIG_ERR)
        ::signal(signo, old_handler);
#endif

    _old_handler_map.erase(iter);
    return true;
}

}

void InstallSignals()
{
    //! 要禁止信号触发
    sigset_t new_sigmask, old_sigmask;
    sigfillset(&new_sigmask);
    sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    SetScopeExitAction([&] { sigprocmask(SIG_SETMASK, &old_sigmask, 0); });

    //! 注册信号处理函数
    InstallSignal(SIGSEGV, true);
    InstallSignal(SIGILL,  true);
    InstallSignal(SIGABRT, true);
    InstallSignal(SIGBUS,  true);
    InstallSignal(SIGPIPE, false);
}

void UninstallSignals()
{
    //! 要禁止信号触发
    sigset_t new_sigmask, old_sigmask;
    sigfillset(&new_sigmask);
    sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    SetScopeExitAction([&] { sigprocmask(SIG_SETMASK, &old_sigmask, 0); });

    UninstallSignal(SIGSEGV);
    UninstallSignal(SIGILL);
    UninstallSignal(SIGABRT);
    UninstallSignal(SIGBUS);
    UninstallSignal(SIGPIPE);
}

}
}
