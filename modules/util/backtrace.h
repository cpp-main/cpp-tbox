#ifndef TBOX_UTIL_BACKTRACE_H_20220708
#define TBOX_UTIL_BACKTRACE_H_20220708

#include <string>
#include <tbox/base/log.h>

namespace tbox {
namespace util {

std::string DumpCallStack(const unsigned int max_frames = 32);

}
}

#define LogCallStack() LogTrace("call stack:\n%s", tbox::util::DumpCallStack().c_str())

#endif // TBOX_UTIL_BACKTRACE_H_20220708
