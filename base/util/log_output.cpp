#include "log_output.h"

#include <sys/time.h>
#include <unistd.h>

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <mutex>

#define TIMESTAMP_STRING_SIZE   28

namespace {

    const char *level_name[] = { "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE" };
    const char *level_color_start[] = { "\033[31;4m", "\033[31m", "\033[33m", "\033[32m", "\033[34m", "\033[35m" };
    const char *level_color_end = "\033[0m";
    std::mutex _stdout_lock;

    void _GetCurrTimeString(char *timestamp)
    {
        struct timeval tv;
        struct timezone tz;

        gettimeofday(&tv, &tz);
        struct tm tm;
        localtime_r(&tv.tv_sec, &tm);
        char tmp[20];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &tm);
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%s.%06ld", tmp, tv.tv_usec);
    }

    void _PrintLogToStdout(const char *module_id, const char *func_name, const char *file_name,
                           int line, int level, const char *fmt, va_list args)
    {
        char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
        _GetCurrTimeString(timestamp);

        std::lock_guard<std::mutex> lg(_stdout_lock);

        printf("%s", level_color_start[level]);

        printf("%s %ld %s %s ", timestamp, ::syscall(SYS_gettid), level_name[level], module_id);

        if (func_name != NULL)
            printf("%s() ", func_name);

        if (fmt != NULL) {
            vprintf(fmt, args);
            putchar(' ');
        }

        if (file_name != NULL) {
            printf("-- %s:%d", file_name, line);
        }

        printf("%s", level_color_end);
        putchar('\n');
    }
}

extern "C" {
    void LogOutput_Initialize(const char *proc_name)
    {
    }

    void LogOutput_Cleanup()
    {
    }

    void LogOutput_SetMask(int output_mask)
    {
        _output_mask = output_mask;
    }

    void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                       int line, int level, const char *fmt, ...)
    {
        if (level < 0) level = 0;
        if (level > 5) level = 5;

        const char *module_id_be_print = (module_id != NULL) ? module_id : "???";

        va_list args;

        va_start(args, fmt);
        _PrintLogToStdout(module_id_be_print, func_name, file_name, line, level, fmt, args);
        va_end(args);
    }
}
