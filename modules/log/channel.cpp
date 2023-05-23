#include "channel.h"
#include <cstring>
#include <functional>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace tbox {
namespace log {

Channel::~Channel()
{
    disable();
}

void Channel::setLevel(int level, const std::string &module)
{
    std::lock_guard<std::mutex> _lk(lock_);
    if (module.empty())
        default_level_ = level;
    else
        modules_level_[module] = level;
}

void Channel::enableColor(bool enable)
{
    enable_color_ = enable;
}

bool Channel::enable()
{
    using namespace std::placeholders;
    if (output_id_ == 0) {
        output_id_ = LogAddPrintfFunc(HandleLog, this);
        onEnable();
        return true;
    }
    return false;
}

void Channel::disable()
{
    if (output_id_ != 0) {
        onDisable();
        LogRemovePrintfFunc(output_id_);
        output_id_ = 0;
    }
}

bool Channel::filter(int level, const std::string &module)
{
    std::lock_guard<std::mutex> _lk(lock_);
    auto iter = modules_level_.find(module);
    if (iter != modules_level_.end())
        return level <= iter->second;
    else
        return level <= default_level_;
}

void Channel::HandleLog(const LogContent *content, void *ptr)
{
    Channel *pthis = static_cast<Channel*>(ptr);
    pthis->handleLog(content);
}

namespace {
const char *level_name = "FEWNIDT";
const int level_color_num[] = {31, 91, 93, 33, 32, 36, 35};
}

void Channel::handleLog(const LogContent *content)
{
    if (!filter(content->level, content->module_id))
        return;

    size_t buff_size = 1024;    //! 初始大小，可应对绝大数情况

    //! 加循环为了应对缓冲不够的情况
    for (;;) {
        char buff[buff_size];
        size_t pos = 0;

#define REMAIN_SIZE ((buff_size > pos) ? (buff_size - pos) : 0)
#define WRITE_PTR   (buff + pos)

        udpateTimestampStr(content->timestamp.sec);

        size_t len = 0;

        //! 开启色彩，显示日志等级
        if (enable_color_) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "\033[%dm", level_color_num[content->level]);
            pos += len;
        }

        len = snprintf(WRITE_PTR, REMAIN_SIZE, "%c %s.%06u %ld %s ",
                       level_name[content->level],
                       timestamp_str_, content->timestamp.usec,
                       content->thread_id, content->module_id);
        pos += len;

        if (content->func_name != nullptr) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "%s() ", content->func_name);
            pos += len;
        }

        if (content->text_ptr != nullptr) {
            if (REMAIN_SIZE >= content->text_len)
                memcpy(WRITE_PTR, content->text_ptr, content->text_len);
            pos += content->text_len;

            if (REMAIN_SIZE >= 1)    //! 追加一个空格
                *WRITE_PTR = ' ';
            ++pos;
        }

        if (content->file_name != nullptr) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "-- %s:%d", content->file_name, content->line);
            pos += len;
        }

        if (enable_color_) {
            if (REMAIN_SIZE >= 4)
                memcpy(WRITE_PTR, "\033[0m", 4);
            pos += 4;
        }

        if (REMAIN_SIZE >= 1)
            *WRITE_PTR = '\0';  //! 追加结束符
        ++pos;

#undef REMAIN_SIZE
#undef WRITE_PTR

        //! 如果缓冲区是够用的，就完成
        if (pos <= buff_size) {
            onLogFrontEnd(buff, pos);
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

void Channel::udpateTimestampStr(uint32_t sec)
{
    std::lock_guard<std::mutex> lg(lock_);
    if (timestamp_sec_ != sec) {
        time_t ts_sec = sec;
        struct tm tm;
        localtime_r(&ts_sec, &tm);
        strftime(timestamp_str_, sizeof(timestamp_str_), "%m-%d %H:%M:%S", &tm);
        timestamp_sec_ = sec;
    }
}

}
}
