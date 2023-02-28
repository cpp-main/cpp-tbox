#ifndef TBOX_UTIL_BACKTRACE_H_20220708
#define TBOX_UTIL_BACKTRACE_H_20220708

#include <signal.h>
#include <string>
#include <tbox/base/log.h>

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
 *          .maxFrames(24)      // if not set, default is 32
 *          .skipFrames(2)      // if not set, default is 1
 *          .submit({SIGSEGV, SIGABRT}); // submit SIGSEGV and SIGABRT as error
 */
class Backtrace {
  public:
    static Backtrace& instance();

  public:
    Backtrace& maxFrames(unsigned int max);  // default is 32
    Backtrace& skipFrames(unsigned int skip);      // default is 1(will note skip)

    void submit(std::initializer_list<int> signals);

    /*
     *  @brief Generate a human-readable backtrace string
     *
     *  @return backtrace string
     */
    static std::string DumpCallStack(const unsigned int max_frames = 32, const unsigned int skip_frames = 1);

  private:
    static void HandleErrorSignal(int signal_number, siginfo_t *signal_info, void *arg);

  private:
    unsigned int max_frames_  = 32;
    unsigned int skip_frames_ = 1;
};

}
}

#define LogStack() LogTrace("call stack:\n%s", Backtrace::DumpCallStack().c_str())

#endif // TBOX_UTIL_BACKTRACE_H_20220708
