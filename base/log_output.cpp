#include "log_output.h"

#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <syslog.h>
#include <mutex>

#include "log_imp.h"

#define TIMESTAMP_STRING_SIZE   28

namespace {
    int _output_mask = LOG_OUTPUT_MASK_STDOUT | LOG_OUTPUT_MASK_SYSLOG;

    const char *level_name = "FEWNIDT";
    const int level_color_num[] = {31, 91, 93, 33, 39, 36, 35};
    std::mutex _stdout_lock;

    void _GetCurrTimeString(char *timestamp)
    {
        struct timeval tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
#if 1
        struct tm tm;
        localtime_r(&tv.tv_sec, &tm);
        char tmp[20];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &tm);
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%s.%06ld", tmp, tv.tv_usec);
#else
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%ld.%06ld", tv.tv_sec, tv.tv_usec);
#endif
    }

    void _PrintLogToStdout(LogContent *content)
    {
        char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
        _GetCurrTimeString(timestamp);

        std::lock_guard<std::mutex> lg(_stdout_lock);

        //! 开启色彩，显示日志等级
        printf("\033[%dm[%c] ", level_color_num[content->level], level_name[content->level]);

        //! 打印时间戳、线程号、模块名
        printf("%s %ld %s ", timestamp, ::syscall(SYS_gettid), content->module_id);

        if (content->func_name != NULL)
            printf("%s() ", content->func_name);

        if (content->fmt != NULL) {
            va_list args;
            va_copy(args, content->args);
            vprintf(content->fmt, args);    //! 不可以直接使用 content->args，因为 va_list 只能被使用一次
            putchar(' ');
        }

        if (content->file_name != NULL) {
            printf("-- %s:%d", content->file_name, content->line);
        }

        puts("\033[0m");    //! 恢复色彩
    }

    const int loglevel_to_syslog[] = { LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG, LOG_DEBUG };

    //! syslogd
    void _PrintLogToSyslog(LogContent *content)
    {
        const int buff_size = 2048;
        char buff[buff_size];

        int write_size = buff_size;

        int len = snprintf(buff + (buff_size - write_size), write_size, "%ld %s ", syscall(SYS_gettid), content->module_id);
        write_size -= len;

        if (write_size > 2 && content->level == 5) {
            len = snprintf(buff + (buff_size - write_size), write_size, "==TRACE== ");
            write_size -= len;
        }

        if (write_size > 2 && content->func_name != NULL) {
            len = snprintf(buff + (buff_size - write_size), write_size, "%s() ", content->func_name);
            write_size -= len;
        }

        if (write_size > 2 && content->fmt != NULL) {
            va_list args;
            va_copy(args, content->args);   //! 同上，va_list 要被复制了使用
            len = vsnprintf(buff + (buff_size - write_size), write_size, content->fmt, args);
            write_size -= len;

            if (write_size > 2) {
                buff[(buff_size - write_size)] = ' ';
                buff[(buff_size - write_size) + 1] = '\0';
                write_size -= 1;
            }
        }

        if (write_size > 2 && content->file_name != NULL) {
            len = snprintf(buff + (buff_size - write_size), write_size, "-- %s:%d", content->file_name, content->line);
            write_size -= len;
        }

        syslog(loglevel_to_syslog[content->level], "%s", buff);
    }
}

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(LogContent *content);

extern "C" {

    static void _LogOutput_PrintfFunc(LogContent *content);

    void LogOutput_Initialize(const char *proc_name)
    {
        LogSetPrintfFunc(_LogOutput_PrintfFunc);
        openlog(proc_name, 0, LOG_USER);
    }

    void LogOutput_Cleanup()
    {
        closelog();
        LogSetPrintfFunc(nullptr);
    }

    void LogOutput_SetMask(int output_mask)
    {
        _output_mask = output_mask;
    }

    static void _LogOutput_PrintfFunc(LogContent *content)
    {
        if ((LogOutput_FilterFunc != NULL) &&
            !LogOutput_FilterFunc(content))
            return;

        if (_output_mask & LOG_OUTPUT_MASK_STDOUT)
            _PrintLogToStdout(content);

        if (_output_mask & LOG_OUTPUT_MASK_SYSLOG)
            _PrintLogToSyslog(content);
    }
}
