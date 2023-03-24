#ifndef TBOX_UTIL_SAFE_EXECUTE_H_20230324
#define TBOX_UTIL_SAFE_EXECUTE_H_20230324

#include <functional>

namespace tbox {
namespace util {

//! SafeExecute 执行所需选项
enum SafeExecuteFlags {
    SAFE_EXECUTE_PRINT_STACK = 0x01,    //!< 打印调用栈
    SAFE_EXECUTE_ABORT = 0x02           //!< 终止进程
};

/// 安全执行，捕获所有异常，确保无异常抛出
/**
 * \param func      将要执行的函数，通常是lambda
 * \param flags     选项 SafeRunFlag
 *
 * \return true     运行过程中没有出现异常
 * \return false    运行过程中捕获到了异常
 */
bool SafeExecute(const std::function<void()> &func, int flags = 0);

/// 安全执行，不打印任何日志
/**
 * \param func      将要执行的函数，通常是lambda
 *
 * \return true     运行过程中没有出现异常
 * \return false    运行过程中捕获到了异常
 */
bool SafeExecuteQuietly(const std::function<void()> &func);

}
}

#endif // TBOX_UTIL_SAFE_EXECUTE_H_20230324
