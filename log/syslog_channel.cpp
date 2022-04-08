#include "syslog_channel.h"
#include <syslog.h>

namespace tbox {
namespace log {

#define TIMESTAMP_STRING_SIZE   28

namespace {
const int loglevel_to_syslog[] = { LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG, LOG_DEBUG };
}

void SyslogChannel::onLogFrontEnd(LogContent *content)
{
    const int buff_size = 2048;
    char buff[buff_size];

    int write_size = buff_size;

    int len = snprintf(buff + (buff_size - write_size), write_size, "%ld %s ", content->thread_id, content->module_id);
    write_size -= len;

    if (write_size > 2 && content->level == 5) {
        len = snprintf(buff + (buff_size - write_size), write_size, "==TRACE== ");
        write_size -= len;
    }

    if (write_size > 2 && content->func_name != nullptr) {
        len = snprintf(buff + (buff_size - write_size), write_size, "%s() ", content->func_name);
        write_size -= len;
    }

    if (write_size > 2 && content->fmt != nullptr) {
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

    if (write_size > 2 && content->file_name != nullptr) {
        len = snprintf(buff + (buff_size - write_size), write_size, "-- %s:%d", content->file_name, content->line);
        write_size -= len;
    }

    syslog(loglevel_to_syslog[content->level], "%s", buff);
}

}
}
