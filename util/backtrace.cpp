#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "backtrace.h"

namespace tbox {
namespace util {

Backtrace& Backtrace::instance()
{
    static Backtrace ins;
    return ins;
}

void Backtrace::submit(std::initializer_list<int> signals)
{
    /// catch segment fault action
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = Backtrace::HandleErrorSignal;
    sa.sa_flags = SA_SIGINFO;

    for (auto iter = signals.begin(); iter != signals.end(); ++iter)
        sigaction(*iter, &sa, nullptr);
}

Backtrace& Backtrace::maxFrames(unsigned int max)
{
    max_frames_ = max;
    return *this;
}

Backtrace& Backtrace::skipFrames(unsigned int skip)
{
    skip_frames_ = skip;
    return *this;
}

void Backtrace::HandleErrorSignal(int signal_number, siginfo_t *signal_info, void *arg)
{
    (void) signal_info;
    (void) arg;

    std::cerr << "catch signal:" << signal_number << ", call-stack dumped:" << std::endl;
    std::string dumpinfo = Backtrace::DumpCallStack(Backtrace::instance().max_frames_, Backtrace::instance().skip_frames_);
    std::cerr << dumpinfo << std::endl;

    _exit(signal_number);
}


std::string Backtrace::DumpCallStack(const unsigned int max_frames, const unsigned int skip_frames)
{
    char buf[1024];
    Dl_info info;

    void *callstack[max_frames];
    std::ostringstream oss;

    unsigned int number_frames = ::backtrace(callstack, max_frames);
    char **symbols = ::backtrace_symbols(callstack, number_frames);

    constexpr const char *BACKTRACE_FORMAT = "[%d]: %s <%p>"; // callstack number | address | symbols

    for (unsigned int i = skip_frames; i < number_frames; ++i) {
        /// try to translate an address to symbolic information
        if (dladdr(callstack[i], &info)) {
            int status = -1;
            char *demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            snprintf(buf, sizeof(buf), BACKTRACE_FORMAT, i, (status == 0 ? demangled : info.dli_sname), callstack[i]);
            if (demangled != nullptr)
                free(demangled);
        } else {
            snprintf(buf, sizeof(buf), BACKTRACE_FORMAT, i, (symbols != nullptr ? symbols[i] : "null"), callstack[i]);
        }

        oss << buf << std::endl;
    }

    if (symbols != nullptr)
        free(symbols);

    if (number_frames >= max_frames)
        oss << "[truncated]" << std::endl;

    return oss.str();
}

}
}
