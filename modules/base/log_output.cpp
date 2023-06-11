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

#include "log_imp.h"

#define TIMESTAMP_STRING_SIZE   28
#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace {
    const char *level_name = "FEWNIDT";
    const int level_color_num[] = {31, 91, 93, 33, 32, 36, 35};
    std::mutex _stdout_lock;

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
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%u.%06u", content->timestamp.sec, content->timestamp.usec);
#endif
    }

    void _PrintLogToStdout(const LogContent *content)
    {
        char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
        _GetCurrTimeString(content, timestamp);

        std::lock_guard<std::mutex> lg(_stdout_lock);

        //! 开启色彩，显示日志等级
        printf("\033[%dm%c ", level_color_num[content->level], level_name[content->level]);

        //! 打印时间戳、线程号、模块名
        printf("%s %ld %s ", timestamp, content->thread_id, content->module_id);

        if (content->func_name != nullptr)
            printf("%s() ", content->func_name);

        printf("%s ", content->text_ptr);

        if (content->file_name != nullptr) {
            printf("-- %s:%d", content->file_name, content->line);
        }

        puts("\033[0m");    //! 恢复色彩
    }
}

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(const LogContent *content);

extern "C" {

    static void _LogOutput_PrintfFunc(const LogContent *content, void *ptr);
    static uint32_t _id = 0;

    void LogOutput_Initialize()
    {
        _id = LogAddPrintfFunc(_LogOutput_PrintfFunc, nullptr);
    }

    void LogOutput_Cleanup()
    {
        LogRemovePrintfFunc(_id);
        _id = 0;
    }

    static void _LogOutput_PrintfFunc(const LogContent *content, void *ptr)
    {
        if ((LogOutput_FilterFunc != nullptr) &&
            !LogOutput_FilterFunc(content))
            return;

        _PrintLogToStdout(content);

        (void)ptr;
    }
}
