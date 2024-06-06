/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
//! tbox/base/log.h
#ifndef TBOX_LOG_H_20170512
#define TBOX_LOG_H_20170512

//! Define log levels
#define LOG_LEVEL_FATAL     0   //!< Program will crash
#define LOG_LEVEL_ERROR     1   //!< Got serious problem, program can't handle
#define LOG_LEVEL_WARN      2   //!< Got inner abnormal situation, but program can handle it
#define LOG_LEVEL_NOTICE    3   //!< It not big problem, but we should notice it, such as invalid data input
#define LOG_LEVEL_IMPORTANT 4   //!< Important message
#define LOG_LEVEL_INFO      5   //!< Normal message exchange with other program
#define LOG_LEVEL_DEBUG     6   //!< Normal process inside program
#define LOG_LEVEL_TRACE     7   //!< Temporary debugging log
#define LOG_LEVEL_MAX       8   //!< MAX

//! Module ID
#ifndef LOG_MODULE_ID
    #ifdef MODULE_ID
        #define LOG_MODULE_ID MODULE_ID
    #else
        #warning "Please define LOG_MODULE_ID as your module name, otherwise it will be ???"
        #define LOG_MODULE_ID   "???"
    #endif
#endif //LOG_MODULE_ID

//! Define commonly macros
#define LogPrintf(level, fmt, ...) \
    LogPrintfFunc(LOG_MODULE_ID, __func__, __FILE__, __LINE__, level, 1, fmt, ## __VA_ARGS__)

#define LogPuts(level, text) \
    LogPrintfFunc(LOG_MODULE_ID, __func__, __FILE__, __LINE__, level, 0, text)

#define LogFatal(fmt, ...)      LogPrintf(LOG_LEVEL_FATAL,  fmt, ## __VA_ARGS__)
#define LogErr(fmt, ...)        LogPrintf(LOG_LEVEL_ERROR,  fmt, ## __VA_ARGS__)
#define LogWarn(fmt, ...)       LogPrintf(LOG_LEVEL_WARN,   fmt, ## __VA_ARGS__)
#define LogNotice(fmt, ...)     LogPrintf(LOG_LEVEL_NOTICE, fmt, ## __VA_ARGS__)
#define LogImportant(fmt, ...)  LogPrintf(LOG_LEVEL_IMPORTANT, fmt, ## __VA_ARGS__)
#define LogInfo(fmt, ...)       LogPrintf(LOG_LEVEL_INFO,   fmt, ## __VA_ARGS__)

#if !defined(STATIC_LOG_LEVEL) || (STATIC_LOG_LEVEL >= LOG_LEVEL_DEBUG)
    #define LogDbg(fmt, ...)    LogPrintf(LOG_LEVEL_DEBUG, fmt, ## __VA_ARGS__)
#else
    #define LogDbg(fmt, ...)
#endif

#if !defined(STATIC_LOG_LEVEL) || (STATIC_LOG_LEVEL >= LOG_LEVEL_TRACE)
    #define LogTrace(fmt, ...)  LogPrintf(LOG_LEVEL_TRACE, fmt, ## __VA_ARGS__)
    #define LogTag()            LogPuts(LOG_LEVEL_TRACE, "==> Run Here <==")
#else
    #define LogTrace(fmt, ...)
    #define LogTag()
#endif

#define LogUndo() LogPuts(LOG_LEVEL_NOTICE, "!!! Undo !!!")

//! 打印错误码，需要 #include <string.h>
#define LogErrno(err, fmt, ...) LogErr("Errno:%d(%s) " fmt, (err), strerror(err), ## __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief  Log print function
//!
//! \param  module_id   Module Id
//! \param  func_name   Function name
//! \param  file_name   File name
//! \param  line        Code line
//! \param  level       Log level
//! \param  with_args   Whether with args
//! \param  fmt         Log format string
//!
//! \note   We only declare this function here.
//!         It's your duty to implement it.
//!
void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, int with_args, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //TBOX_LOG_H_20170512
