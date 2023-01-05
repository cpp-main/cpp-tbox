#include <signal.h>
#include <execinfo.h>

#include <iostream>
#include <sstream>
#include <functional>

#include <tbox/base/log.h>
#include <tbox/util/backtrace.h>

int main(int argc, char **argv);

namespace tbox {
namespace main {

extern std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

namespace {

//! 处理程序运行异常信号
void OnErrorSignal(int signo)
{
    const std::string &stack_str = util::Backtrace::DumpCallStack(32, 0);

    LogFatal("Receive signal %d", signo);
    LogFatal("main: <%p>\n-----call stack-----\n%s",
      ::main, stack_str.c_str());

    if (error_exit_func)    //! 执行一些善后处理
        error_exit_func();

    signal(SIGABRT, SIG_DFL);
    std::abort();
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
