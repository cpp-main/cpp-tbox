/**
 * 重定义宏 assert(), 令其它错误信息打印到Log日志上
 */
#ifndef TBOX_BASE_ASSERT_H_20221026
#define TBOX_BASE_ASSERT_H_20221026

#include <cstdlib>
#include "log.h"

//! WARN: Don't use this in log related module

#undef assert

#ifdef  NDEBUG //! 在非调试模式下，什么都不用做
# define assert(expr) void(0)

#else //! 在调试模式下，要在日志中打印错误并退出

# define assert(expr) \
  if (!static_cast<bool>(expr)) { \
    LogFatal("assert(%s)", #expr); \
    std::abort(); \
  }

#endif

#endif //TBOX_BASE_ASSERT_H_20221026
