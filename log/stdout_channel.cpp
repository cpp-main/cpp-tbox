#include "stdout_channel.h"
#include <sys/time.h>

namespace tbox {
namespace log {

#define TIMESTAMP_STRING_SIZE   28

namespace {
const char *level_name = "FEWNIDT";
const int level_color_num[] = {31, 91, 93, 33, 32, 36, 35};

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
}

void StdoutChannel::onLogFrontEnd(LogContent *content)
{
    char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
    _GetCurrTimeString(content, timestamp);

    std::lock_guard<std::mutex> lg(lock_);

    if (enable_color_)
        printf("\033[%dm", level_color_num[content->level]);

    //! 开启色彩，显示日志等级
    printf("<%c> ", level_name[content->level]);

    //! 打印时间戳、线程号、模块名
    printf("%s %ld %s ", timestamp, content->thread_id, content->module_id);

    if (content->func_name != nullptr)
        printf("%s() ", content->func_name);

    if (content->fmt != nullptr) {
        va_list args;
        va_copy(args, content->args);
        vprintf(content->fmt, args);    //! 不可以直接使用 content->args，因为 va_list 只能被使用一次
        putchar(' ');
    }

    if (content->file_name != nullptr) {
        printf("-- %s:%d", content->file_name, content->line);
    }

    if (enable_color_)
        puts("\033[0m");    //! 恢复色彩
    else
        putchar('\n');
}

}
}
