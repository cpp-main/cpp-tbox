#include "log_imp.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

//! 日志输出函数指针
static LogPrintfFuncType _log_printf_func = nullptr;

/**
 * \brief   日志格式化打印接口的实现
 *
 * 1.对数据合法性进行校验;
 * 2.将日志数据打包成 LogContent，然后调用 _log_printf_func 指向的函数进行输出
 */
void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < 0) level = 0;
    if (level > LOG_LEVEL_TRACE) level = LOG_LEVEL_TRACE;

    const char *module_id_be_print = (module_id != nullptr) ? module_id : "???";

    LogContent content = {
        .module_id = module_id_be_print,
        .func_name = func_name,
        .file_name = file_name,
        .line = line,
        .level = level,
        .fmt = fmt
    };
    va_copy(content.args, args);    //! va_list 不能直接赋值，需要使用 va_copy()

    if (_log_printf_func != nullptr)
        _log_printf_func(&content);

    va_end(args);
}

void LogSetPrintfFunc(LogPrintfFuncType func)
{
    _log_printf_func = func;
}

#ifdef __cplusplus
}
#endif
