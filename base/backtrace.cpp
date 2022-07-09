#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include "backtrace.h"

static uint32_t g_max_frames = 32;
static uint32_t g_skip_frames = 1;

Backtrace::Backtrace()
{ }

Backtrace::~Backtrace() {}

Backtrace& Backtrace::instance()
{
    static Backtrace ins;
    return ins;
}

void Backtrace::submit()
{
    for (const auto &it : signal_list_)
        signal(it.signal_number, it.handler);

    std::vector<SignalNode> tmp;
    std::swap(tmp, signal_list_);

    /// catch segment fault action
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = Backtrace::handleSigsegv;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, nullptr);
}

Backtrace& Backtrace::capture(int signo, signal_handler_t handler)
{
    signal_list_.push_back(SignalNode(signo, handler));
    return *this;
}

Backtrace& Backtrace::maxFrames(unsigned int max_frames)
{
    g_max_frames = max_frames;
    return *this;
}

Backtrace& Backtrace::skip(unsigned int skip)
{
    g_skip_frames = skip;
    return *this;
}

void Backtrace::handleSigsegv(int signal_number, siginfo_t *signal_info, void *arg)
{
    (void) signal_number;
    (void) signal_info;
    (void) arg;

    std::cerr << "call-stack dumped:" << std::endl;
    std::string dumpinfo =  Backtrace::backtraceString(g_max_frames, g_skip_frames);
    std::cerr << dumpinfo << std::endl;
    _exit(signal_number);
}


std::string Backtrace::backtraceString(const int max_frames, const int skip)
{
    void *callstack[max_frames];
    char buf[1024] = { 0 };
    std::ostringstream oss;

    int number_frames = ::backtrace(callstack, max_frames);
    char **symbols = ::backtrace_symbols(callstack, number_frames);

    for (int i = skip; i < number_frames; ++i) {
        Dl_info info;
        /// try to translate an address to symbolic information
        if (dladdr(callstack[i], &info)) {
            char *demangled = nullptr;
            int status = -1;
            demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            snprintf(buf, sizeof(buf), BACKTRACE_FORMAT, i, callstack[i], status == 0 ? demangled : info.dli_sname);
            if (demangled) free(demangled);
        } else
            snprintf(buf, sizeof(buf), BACKTRACE_FORMAT, i, callstack[i], symbols[i]);

        oss << buf;
    }

    if (symbols)
        free(symbols);

    if (number_frames >= max_frames)
        oss << "[truncated]\n";

    return oss.str();
}
