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

#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <mutex>
#include <iostream>

#include "log_impl.h"

#define TIMESTAMP_STRING_SIZE   27

namespace {

    void _GetCurrTimeString(const LogContent *content, char *timestamp)
    {
#if 1
        time_t ts_sec = content->timestamp.sec;
        struct tm tm;
        localtime_r(&ts_sec, &tm);
        char tmp[20];
        strftime(tmp, sizeof(tmp), "%F %H:%M:%S", &tm);
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%s.%06u", tmp, content->timestamp.usec);
#else
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%u.%06u", content->timestamp.sec, content->timestamp.usec);
#endif
    }

    void _PrintLogToStdout(const LogContent *content)
    {
        char timestamp[TIMESTAMP_STRING_SIZE]; //!  "05-13 23:45:07.000000"
        _GetCurrTimeString(content, timestamp);

        //! 打印色彩、等级、时间戳、线程号、模块名
        printf("\033[%sm%c %s %ld %s ",
            LOG_LEVEL_COLOR_CODE[content->level], LOG_LEVEL_LEVEL_CODE[content->level],
            timestamp, content->thread_id, content->module_id);

        if (content->func_name != nullptr)
            printf("%s() ", content->func_name);

        if (content->text_len > 0)
            printf("%.*s ", content->text_len, content->text_ptr);

        if (content->text_trunc == 1)
            printf("(TRUNCATED) ");

        if (content->file_name != nullptr)
            printf("-- %s:%d", content->file_name, content->line);

        puts("\033[0m");    //! 恢复色彩
    }
}

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(const LogContent *content);

extern "C" {

    static void _LogOutput_PrintfFunc(const LogContent *content, void *ptr);
    static uint32_t _id = 0;

    void LogOutput_Enable()
    {
        if (_id == 0)
            _id = LogAddPrintfFunc(_LogOutput_PrintfFunc, nullptr);
    }

    void LogOutput_Disable()
    {
        LogRemovePrintfFunc(_id);
        _id = 0;
    }

    static void _LogOutput_PrintfFunc(const LogContent *content, void *)
    {
        if ((LogOutput_FilterFunc != nullptr) &&
            !LogOutput_FilterFunc(content))
            return;

        _PrintLogToStdout(content);
    }
}
