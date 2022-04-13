#include "async_channel.h"
#include <cstring>
#include <algorithm>
#include <iostream>

namespace tbox {
namespace log {

AsyncChannel::AsyncChannel()
{
    string_buff_.reserve(1024);
}

AsyncChannel::~AsyncChannel()
{ }

bool AsyncChannel::initialize(const Config &cfg)
{
    if (!async_pipe_.initialize(cfg))
        return false;

    using namespace std::placeholders;
    async_pipe_.setCallback(std::bind(&AsyncChannel::onPipeAppend, this, _1, _2));

    return true;
}

void AsyncChannel::cleanup()
{
    async_pipe_.cleanup();
}

void AsyncChannel::onLogFrontEnd(LogContent *content)
{
    const char *level_name = "FEWNIDT";

    size_t buff_size = 1024;    //! 初始大小，可应对绝大数情况

    //! 加循环为了应对缓冲不够的情况
    for (;;) {
        char buff[buff_size];
        size_t pos = 0;

#define REMAIN_SIZE ((buff_size > pos) ? (buff_size - pos) : 0)
#define WRITE_PTR   (buff + pos)

        udpateTimestampStr(content->timestamp.sec);

        size_t len = snprintf(WRITE_PTR, REMAIN_SIZE, "%c %s.%06u %ld %s ",
                              level_name[content->level],
                              timestamp_str_, content->timestamp.usec,
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
            async_pipe_.append(buff, pos);
            break;
        }

        //! 否则扩展缓冲区，重来
        buff_size = pos;
    }
}

void AsyncChannel::onPipeAppend(const void *data_ptr, size_t data_size)
{
    const char *start_ptr = static_cast<const char *>(data_ptr);
    const char *end_ptr = start_ptr + data_size;

    for (;;) {
        auto zero_ptr = std::find(start_ptr, end_ptr, 0);
        if (zero_ptr != end_ptr) {
            string_buff_ += start_ptr;
            start_ptr = zero_ptr + 1;
            onLogBackEnd(string_buff_);
            string_buff_.clear();
        } else {
            std::for_each(start_ptr, end_ptr,
                [=] (char c) {
                    string_buff_.push_back(c);
                }
            );
            break;
        }
    }
}

void AsyncChannel::udpateTimestampStr(uint32_t sec)
{
    std::lock_guard<std::mutex> lg(lock_);
    if (timestamp_sec_ != sec) {
        time_t ts_sec = sec;
        struct tm tm;
        localtime_r(&ts_sec, &tm);
        strftime(timestamp_str_, sizeof(timestamp_str_), "%Y-%m-%d %H:%M:%S", &tm);
        timestamp_sec_ = sec;
    }
}

}
}
