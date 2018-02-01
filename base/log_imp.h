#ifndef TBOX_BASE_LOG_IMP_20180201
#define TBOX_BASE_LOG_IMP_20180201

/**
 * 本文件只声明了函数 LogSetPrintfFunc() 该函数用于指定日志输出函数
 * 如果不指定，则默认不输出。
 */

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

//! 日志内容
struct LogContent {
    const char *module_id;
    const char *func_name;
    const char *file_name;
    int line;
    int level;
    const char *fmt;
    va_list args;
};

typedef void (*LogPrintfFuncType)(LogContent *content);

//! \brief  Set log output function
void LogSetPrintfFunc(LogPrintfFuncType func);

#ifdef __cplusplus
}
#endif

#endif //TBOX_BASE_LOG_IMP_20180201
