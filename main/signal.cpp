#include <signal.h>
#include <execinfo.h>

#include <iostream>
#include <sstream>
#include <functional>

#include <tbox/base/log.h>
#include <tbox/util/backtrace.h>

namespace tbox {
namespace main {

extern std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

namespace {

//! 打印调用栈
void PrintCallStack()
{
    std::string &&stack_str = util::Backtrace::DumpCallStack(32, 5);
    LogFatal("\n-----call stack-----\n%s", stack_str.c_str());
}

//! 处理程序运行异常信号
void OnErrorSignal(int signo)
{
    LogFatal("Receive signal %d", signo);
    PrintCallStack();
    if (error_exit_func)
        error_exit_func();
    exit(EXIT_FAILURE);
}

void OnWarnSignal(int signo)
{
    LogWarn("Receive signal %d", signo);
}

}

void RegisterSignals()
{
    //! 注册信号处理函数
    signal(SIGSEGV, OnErrorSignal);
    signal(SIGILL,  OnErrorSignal);
    signal(SIGABRT, OnErrorSignal);
    signal(SIGBUS,  OnErrorSignal);
    signal(SIGPIPE, OnWarnSignal);
}

}
}
