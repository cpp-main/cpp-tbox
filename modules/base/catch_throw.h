#ifndef TBOX_CATCH_THROW_H_20230324
#define TBOX_CATCH_THROW_H_20230324

#include <functional>

namespace tbox {

/// 执行函数并捕获所有异常，确保无异常抛出
/**
 * \param func              将要执行的函数，通常是lambda
 * \param print_backtrace   是否打印调用栈
 * \param abort_process     是否结束进程
 *
 * \return false    运行过程中没有出现异常
 * \return true     运行过程中捕获到了异常
 */
bool CatchThrow(const std::function<void()> &func,
                bool print_backtrace = false,
                bool abort_process = false);

/// 同 CatchThrow，但不打印日志、堆栈，也不退出进程
/**
 * \param func              将要执行的函数，通常是lambda
 *
 * \return false    运行过程中没有出现异常
 * \return true     运行过程中捕获到了异常
 */
bool CatchThrowQuietly(const std::function<void()> &func);

}

#endif // TBOX_CATCH_THROW_H_20230324
