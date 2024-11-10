/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <signal.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <functional>
#include <map>

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/backtrace.h>

#define TBOX_USE_SIGACTION

int main(int argc, char **argv);

namespace tbox {
namespace main {

extern void AbnormalExit();

namespace {

#ifdef  TBOX_USE_SIGACTION
using SignalHandler = struct sigaction;
#else
using SignalHandler = void (*) (int);
#endif

std::map<int, SignalHandler> _old_handler_map;

bool _is_recursion_call = false;

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

    if (!_is_recursion_call) {
        _is_recursion_call = true;

        LogFatal("Receive signal %d", signo);

        const std::string &stack_str = DumpBacktrace();
        LogFatal("main: <%p>\n-----call stack-----\n%s", ::main, stack_str.c_str());

        _is_recursion_call = false;

    } else {
        LogFatal("Recursion signal %d", signo);
    }

    AbnormalExit();
}

bool InstallSignal(int signo)
{
    SignalHandler old_handler;
    bool is_succ = false;

#ifdef TBOX_USE_SIGACTION
    struct sigaction new_handler;
    memset(&new_handler, 0, sizeof(new_handler));
    sigemptyset(&new_handler.sa_mask);
    new_handler.sa_sigaction = OnErrorSignal;
    new_handler.sa_flags = SA_SIGINFO;
    ssize_t ret = ::sigaction(signo, &new_handler, &old_handler);
    is_succ = (ret != -1);
#else
    old_handler = ::signal(signo, OnErrorSignal);
    is_succ = (SIG_ERR != old_handler);
#endif

    _old_handler_map[signo] = old_handler;
    return is_succ;
}

}

void InstallErrorSignals()
{
    //! 要禁止信号触发
    sigset_t new_sigmask, old_sigmask;
    sigfillset(&new_sigmask);
    sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    SetScopeExitAction([&] { sigprocmask(SIG_SETMASK, &old_sigmask, 0); });

    //! 注册信号处理函数
    InstallSignal(SIGSEGV);
    InstallSignal(SIGILL);
    InstallSignal(SIGABRT);
    InstallSignal(SIGBUS);
    InstallSignal(SIGFPE);
    InstallSignal(SIGSYS);
}

void UninstallErrorSignals()
{
    //! 要禁止信号触发
    sigset_t new_sigmask, old_sigmask;
    sigfillset(&new_sigmask);
    sigprocmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    SetScopeExitAction([&] { sigprocmask(SIG_SETMASK, &old_sigmask, 0); });

    for (auto &item : _old_handler_map) {
        int signo = item.first;
        auto &old_handler = item.second;

#ifdef TBOX_USE_SIGACTION
        ::sigaction(signo, &old_handler, nullptr);
#else
        if (old_handler != SIG_ERR)
            ::signal(signo, old_handler);
#endif
    }
    _old_handler_map.clear();
}

}
}
