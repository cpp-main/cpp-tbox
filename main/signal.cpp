#include <signal.h>
#include <execinfo.h>

#include <iostream>
#include <sstream>
#include <functional>

#include <tbox/base/log.h>

namespace tbox {
namespace main {

extern std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

namespace {

//! 打印调用栈
void PrintCallStack()
{
    const int buffer_size = 1024;

    void *buffer[buffer_size];
    int n = backtrace(buffer, buffer_size);

    std::stringstream ss;
    char **symbols = backtrace_symbols(buffer, n);
    if (symbols != NULL) {
        for (int i = 0; i < n; i++)
            ss << '[' << i << "] " << symbols[i] << std::endl;
        free(symbols);
    } else {
        ss << "<no stack symbols>" << std::endl;
    }

    LogFatal("\n-----call stack-----\n%s", ss.str().c_str());
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
