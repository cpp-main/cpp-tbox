#include "log_imp.h"

#ifdef __cplusplus
extern "C" {
#endif

static LogPrintfFuncType _log_printf_func = nullptr;

void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < 0) level = 0;
    if (level > 5) level = 5;

    const char *module_id_be_print = (module_id != nullptr) ? module_id : "???";

    LogContent content = { module_id_be_print, func_name, file_name, line, level, fmt };
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
