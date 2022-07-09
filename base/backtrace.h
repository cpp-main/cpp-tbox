#ifndef _BACKTRACE_H
#define _BACKTRACE_H

#include <signal.h>
#include <string>
#include <vector>

#define BACKTRACE_FORMAT "%-2d: %p\t%s\n" // callstack number | address | symbols

typedef void(*signal_handler_t)(int);

/*
 * @brief - Aims to capture signals and formart human-readable backtrace info if
 *          recved SIGSEGV signal
 * @note  - You should add -ldl and -rdynamic to your link parameters
 *
 * # Examples:
 *
 *  Backtrace::instance()
 *          .maxFrames(24) // if not set, default is 32
 *          .skip(2) // if not set, default is 1
 *          .capture(SIGTERM, handle_term)
 *          .capture(SIGABRT, handle_abort)
 *          .capture(SIGINT,  handle_interrupt)
 *          .submit();
 */
class Backtrace
{
private:
    Backtrace();
    ~Backtrace();

public:
    static Backtrace& instance();

public:
    Backtrace& capture(int signal_no, signal_handler_t handler);
    Backtrace& maxFrames(unsigned int max_frames); // default is 32
    Backtrace& skip(unsigned int skip);      // default is 1(will note skip)

    void submit();

private:
    static void handleSigsegv(int signal_number, siginfo_t *signal_info, void *arg);

    /*
     *  @brief Generate a human-readable backtrace string
     *
     *  @return backtrace string
     */
    static std::string backtraceString(const int max_frames = 32, const int skip = 1);

private:
    struct SignalNode
    {
        SignalNode(int s,signal_handler_t h) : signal_number(s), handler(h) { }
        int signal_number;
        signal_handler_t handler;
    };

    std::vector<SignalNode> signal_list_;
};

#endif // _BACKTRACE_H
