#ifndef _BACKTRACE_H
#define _BACKTRACE_H

#include <string>
#include <signal.h>

namespace tbox {
namespace util {

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
 *          .submit();
 */
class Backtrace {
  public:
    static Backtrace& instance();

  public:
    Backtrace& maxFrames(unsigned int max);  // default is 32
    Backtrace& skip(unsigned int skip);      // default is 1(will note skip)

    void submit(std::initializer_list<int> signals);

    /*
     *  @brief Generate a human-readable backtrace string
     *
     *  @return backtrace string
     */
    static std::string DumpCallStack(const unsigned int max_frames, const unsigned int skip = 1);

  private:
    static void HandleErrorSignal(int signal_number, siginfo_t *signal_info, void *arg);

  private:
    unsigned int max_frames_  = 32;
    unsigned int skip_frames_ = 1;
};

}
}

#endif // _BACKTRACE_H
