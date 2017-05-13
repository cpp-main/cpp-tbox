//! tbox/log.h
#ifndef TBOX_LOG_H_20170512
#define TBOX_LOG_H_20170512

#include <stdlib.h>

//! Define log levels
#define LOG_LEVEL_FATAL     0
#define LOG_LEVEL_ERROR     1
#define LOG_LEVEL_WARN      2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_DEBUG     4
#define LOG_LEVEL_TRACE     5

//! Module ID
#ifndef LOG_MODULE_ID
#warning "Please define LOG_MODULE_ID as your module name, otherwise it will be NULL"
#define LOG_MODULE_ID   NULL
#endif //LOG_MODULE_ID

//! Define commonly macros
#define LogPrintf(level, fmt, ...) \
    LogPrintfFunc(LOG_MODULE_ID, __func__, __FILE__, __LINE__, level, fmt, ## __VA_ARGS__)

#define LogFatal(fmt, ...)  LogPrintf(LOG_LEVEL_FATAL, fmt, ## __VA_ARGS__)
#define LogErr(fmt, ...)    LogPrintf(LOG_LEVEL_ERROR, fmt, ## __VA_ARGS__)
#define LogWarn(fmt, ...)   LogPrintf(LOG_LEVEL_WARN,  fmt, ## __VA_ARGS__)
#define LogInfo(fmt, ...)   LogPrintf(LOG_LEVEL_INFO,  fmt, ## __VA_ARGS__)

#if !defined(BUILD_LOG_LEVEL) || (BUILD_LOG_LEVEL >= LOG_LEVEL_DEBUG)
    #define LogDbg(fmt, ...)    LogPrintf(LOG_LEVEL_DEBUG, fmt, ## __VA_ARGS__)
#else
    #define LogDbg(fmt, ...)
#endif

#if !defined(BUILD_LOG_LEVEL) || (BUILD_LOG_LEVEL >= LOG_LEVEL_TRACE)
    #define LogTrace(fmt, ...)  LogPrintf(LOG_LEVEL_TRACE, fmt, ## __VA_ARGS__)
#else
    #define LogTrace(fmt, ...)
#endif

//!
//! \brief  Log print function
//!
//! \param  module_id   Module Id
//! \param  func_name   Function name
//! \param  file_name   File name
//! \param  line        Code line
//! \param  level       Log level
//! \param  fmt         Log format string
//!
//! \note   We only declare this function here.
//!         It's your duty to implement it.
//!
void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, const char *fmt, ...);

#endif //TBOX_LOG_H_20170512
