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

    void _GetCurrTimeString(const LogContent *content, char *timestamp)
    {
#if 1
        time_t ts_sec = content->timestamp.sec;
        struct tm tm;
        localtime_r(&ts_sec, &tm);
        char tmp[20];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &tm);
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%s.%06u", tmp, content->timestamp.usec);
#else
        snprintf(timestamp, TIMESTAMP_STRING_SIZE, "%u.%06u", content->timestamp.sec, content->timestamp.usec);
#endif
    }

    void _PrintLogToStdout(LogContent *content)
    {
        char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
        _GetCurrTimeString(content, timestamp);

        std::lock_guard<std::mutex> lg(_stdout_lock);

        //! 开启色彩，显示日志等级
        printf("\033[%dm<%c> ", level_color_num[content->level], level_name[content->level]);

        //! 打印时间戳、线程号、模块名
        printf("%s %ld %s ", timestamp, content->thread_id, content->module_id);

        if (content->func_name != nullptr)
            printf("%s() ", content->func_name);

        if (content->fmt != nullptr) {
            va_list args;
            va_copy(args, content->args);
            vprintf(content->fmt, args);    //! 不可以直接使用 content->args，因为 va_list 只能被使用一次
            putchar(' ');
        } else {
            printf("%s ", content->fmt);
        }

        if (content->file_name != nullptr) {
            printf("-- %s:%d", content->file_name, content->line);
        }

        puts("\033[0m");    //! 恢复色彩
    }

    //! syslogd
    void _PrintLogToSyslog(LogContent *content)
    {
        const char *level_name = "FEWNIDT";
        const int loglevel_to_syslog[] = { LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG, LOG_DEBUG };

        size_t buff_size = 1024;    //! 初始大小，可应对绝大数情况

        //! 加循环为了应对缓冲不够的情况
        for (;;) {
            char buff[buff_size];
            size_t pos = 0;

#define REMAIN_SIZE ((buff_size > pos) ? (buff_size - pos) : 0)
#define WRITE_PTR   (buff + pos)

            size_t len = snprintf(WRITE_PTR, REMAIN_SIZE, "%c %u.%06u %ld %s ",
                                  level_name[content->level],
                                  content->timestamp.sec, content->timestamp.usec,
                                  content->thread_id, content->module_id);
            pos += len;

            if (content->func_name != nullptr) {
                size_t len = snprintf(WRITE_PTR, REMAIN_SIZE, "%s() ", content->func_name);
                pos += len;
            }

            if (content->fmt != nullptr) {
                if (content->with_args) {
                    va_list args;
                    va_copy(args, content->args);    //! 同上，va_list 要被复制了使用
                    size_t len = vsnprintf(WRITE_PTR, REMAIN_SIZE, content->fmt, args);
                    pos += len;
                } else {
                    size_t len = strlen(content->fmt);
                    if (REMAIN_SIZE >= len)
                        memcpy(WRITE_PTR, content->fmt, len);
                    pos += len;
                }

                if (REMAIN_SIZE >= 1)    //! 追加一个空格
                    *WRITE_PTR = ' ';
                ++pos;
            }

            if (content->file_name != nullptr) {
                size_t len = snprintf(WRITE_PTR, REMAIN_SIZE, "-- %s:%d", content->file_name, content->line);
                pos += len;
            }

            if (REMAIN_SIZE >= 1)
                *WRITE_PTR = '\0';  //! 追加结束符
            ++pos;

#undef REMAIN_SIZE
#undef WRITE_PTR

            //! 如果缓冲区是够用的，就完成
            if (pos <= buff_size) {
                syslog(loglevel_to_syslog[content->level], "%s", buff);
                break;
            }

            //! 否则扩展缓冲区，重来
            buff_size = pos;
        }
    }
}

//! Declare log filte function as weak reference
//! Even if user did't implement their own filter function, it stall works.
bool __attribute((weak)) LogOutput_FilterFunc(LogContent *content);

extern "C" {

    static void _LogOutput_PrintfFunc(LogContent *content, void *ptr);
    static uint32_t _id = 0;

    void LogOutput_Initialize(const char *proc_name)
    {
        _id = LogAddPrintfFunc(_LogOutput_PrintfFunc, nullptr);
        openlog(proc_name, 0, LOG_USER);
    }

    void LogOutput_Cleanup()
    {
        closelog();
        LogRemovePrintfFunc(_id);
        _id = 0;
    }

    void LogOutput_SetMask(int output_mask)
    {
        _output_mask = output_mask;
    }

    static void _LogOutput_PrintfFunc(LogContent *content, void *ptr)
    {
        if ((LogOutput_FilterFunc != nullptr) &&
            !LogOutput_FilterFunc(content))
            return;

        if (_output_mask & LOG_OUTPUT_MASK_STDOUT)
            _PrintLogToStdout(content);

        if (_output_mask & LOG_OUTPUT_MASK_SYSLOG)
            _PrintLogToSyslog(content);
    }
}
