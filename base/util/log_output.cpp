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

#define TIMESTAMP_STRING_SIZE   28

namespace {
    int _output_mask = LOG_OUTPUT_MASK_STDOUT | LOG_OUTPUT_MASK_SYSLOG;

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

    const int loglevel_to_syslog[] = { LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG };

    //! syslogd
    void _PrintLogToSyslog(const char *module_id, const char *func_name, const char *file_name,
                           int line, int level, const char *fmt, va_list args)
    {
        const int buff_size = 2048;
        char buff[buff_size];

        int write_size = buff_size;

        int len = snprintf(buff + (buff_size - write_size), write_size, "%ld %s ", syscall(SYS_gettid), module_id);
        write_size -= len;

        if (write_size > 2 && level == 5) {
            len = snprintf(buff + (buff_size - write_size), write_size, "==TRACE== ");
            write_size -= len;
        }

        if (write_size > 2 && func_name != NULL) {
            len = snprintf(buff + (buff_size - write_size), write_size, "%s() ", func_name);
            write_size -= len;
        }

        if (write_size > 2 && fmt != NULL) {
            len = vsnprintf(buff + (buff_size - write_size), write_size, fmt, args);
            write_size -= len;

            if (write_size > 2) {
                buff[(buff_size - write_size)] = ' ';
                buff[(buff_size - write_size) + 1] = '\0';
                write_size -= 1;
            }
        }

        if (write_size > 2 && file_name != NULL) {
            len = snprintf(buff + (buff_size - write_size), write_size, "-- %s:%d", file_name, line);
            write_size -= len;
        }

        syslog(loglevel_to_syslog[level], "%s", buff);
    }
}

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(const char *module_id, const char *func_name, const char *file_name, int level);

extern "C" {
    void LogOutput_Initialize(const char *proc_name)
    {
        openlog(proc_name, 0, LOG_USER);
    }

    void LogOutput_Cleanup()
    {
        closelog();
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

        if ((LogOutput_FilterFunc != NULL) &&
            !LogOutput_FilterFunc(module_id, func_name, file_name, level))
            return;

        const char *module_id_be_print = (module_id != NULL) ? module_id : "???";

        va_list args;

        va_start(args, fmt);
        if (_output_mask & LOG_OUTPUT_MASK_STDOUT)
            _PrintLogToStdout(module_id_be_print, func_name, file_name, line, level, fmt, args);
        va_end(args);

        va_start(args, fmt);
        if (_output_mask & LOG_OUTPUT_MASK_SYSLOG)
            _PrintLogToSyslog(module_id_be_print, func_name, file_name, line, level, fmt, args);
        va_end(args);
    }
}
