#ifndef TBOX_BACKTRACE_H_20220708
#define TBOX_BACKTRACE_H_20220708

#include <string>
#include "log.h"

namespace tbox {

/// 导出调用栈
std::string DumpBacktrace(const unsigned int max_frames = 64);

}

#define LogBacktrace() LogTrace("call stack:\n%s", tbox::DumpBacktrace().c_str())

#endif // TBOX_BACKTRACE_H_20220708
