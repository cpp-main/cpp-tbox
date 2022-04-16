#include "syslog_channel.h"
#include <cstring>
#include <syslog.h>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace tbox {
namespace log {

void SyslogChannel::onLogFrontEnd(LogContent *content)
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

        if (buff_size > LOG_MAX_LEN) {
            std::cerr << "WARN: log length " << buff_size << ", too long!" << std::endl;
            break;
        }
    }
}

}
}
