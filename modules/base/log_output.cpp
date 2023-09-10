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
#include "log_output.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <mutex>

#include "log_imp.h"

#define TIMESTAMP_STRING_SIZE 22

namespace {
const char *level_name = "FEWNIDT";
const int level_color_num[] = {31, 91, 93, 33, 32, 36, 35};

void _GetCurrTimeString(const LogContent *content, char *timestamp)
{
#if 1
    time_t ts_sec = content->timestamp.sec;
    struct tm tm;
    localtime_r(&ts_sec, &tm);
    char tmp[15];
    strftime(tmp, sizeof(tmp), "%m-%d %H:%M:%S", &tm);
    snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%s.%06u", tmp, content->timestamp.usec);
#else
    snprintf(timestamp,
             TIMESTAMP_STRING_SIZE,
             "%u.%06u",
             content->timestamp.sec,
             content->timestamp.usec);
#endif
}

void _PrintLogToStdout(const LogContent *content)
{
    char timestamp[TIMESTAMP_STRING_SIZE];  //!  "05-13 23:45:07.000000"
    _GetCurrTimeString(content, timestamp);

    //! 开启色彩，显示日志等级
    printf("\033[%dm%c ", level_color_num[content->level], level_name[content->level]);

    //! 打印时间戳、线程号、模块名
    printf("%s %ld %s ", timestamp, content->thread_id, content->module_id);

    if (content->func_name != nullptr) printf("%s() ", content->func_name);

    printf("%s ", content->text_ptr);

    if (content->file_name != nullptr) {
        printf("-- %s:%d", content->file_name, content->line);
    }

    puts("\033[0m");  //! 恢复色彩
}
}  // namespace

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(const LogContent *content);

extern "C" {

static void _LogOutput_PrintfFunc(const LogContent *content, void *ptr);
static uint32_t _id = 0;

void LogOutput_Enable()
{
    if (_id == 0) _id = LogAddPrintfFunc(_LogOutput_PrintfFunc, nullptr);
}

void LogOutput_Disable()
{
    LogRemovePrintfFunc(_id);
    _id = 0;
}

static void _LogOutput_PrintfFunc(const LogContent *content, void *)
{
    if ((LogOutput_FilterFunc != nullptr) && !LogOutput_FilterFunc(content)) return;

    _PrintLogToStdout(content);
}
}
