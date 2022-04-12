#include "async_channel.h"
#include <cstring>
#include <algorithm>
#include <iostream>

namespace tbox {
namespace log {

#define LOG_MAX_LEN 1024

AsyncChannel::AsyncChannel()
{
    string_buff_.reserve(LOG_MAX_LEN);
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

    char buff[LOG_MAX_LEN];
    int remain_size = LOG_MAX_LEN;

#define WRITE_PTR (buff + (LOG_MAX_LEN - remain_size))

#define UPDATE_REMAIN_SIZE() \
    remain_size = (len > remain_size) ? 1 : (remain_size - len)

    udpateTimestampStr(content->timestamp.sec);

    int len = snprintf(WRITE_PTR, remain_size, "<%c> %s.%06u %ld %s ",
                       level_name[content->level],
                       timestamp_str_, content->timestamp.usec,
                       content->thread_id, content->module_id);
    UPDATE_REMAIN_SIZE();

    if (remain_size > 2 && content->func_name != nullptr) {
        int len = snprintf(WRITE_PTR, remain_size, "%s() ", content->func_name);
        UPDATE_REMAIN_SIZE();
    }

    if (remain_size > 2 && content->fmt != nullptr) {
        if (content->with_args) {
            va_list args;
            va_copy(args, content->args);    //! 同上，va_list 要被复制了使用
            int len = vsnprintf(WRITE_PTR, remain_size, content->fmt, args);
            UPDATE_REMAIN_SIZE();
        } else {
            int len = strlen(content->fmt);
            if (len >= remain_size)
                len = remain_size - 1;      //! 要留一个字符放'\0'
            memcpy(WRITE_PTR, content->fmt, len);
            remain_size -= len;
        }

        if (remain_size > 2) {  //! 追加一个空格
            *WRITE_PTR = ' ';
            remain_size -= 1;
        }
    }

    if (remain_size > 2 && content->file_name != nullptr) {
        int len = snprintf(WRITE_PTR, remain_size, "-- %s:%d", content->file_name, content->line);
        UPDATE_REMAIN_SIZE();
    }

    *WRITE_PTR = '\0';  //! 追加结束符
    remain_size -= 1;

#undef UPDATE_REMAIN_SIZE
#undef WRITE_PTR

    async_pipe_.append(buff, (LOG_MAX_LEN - remain_size));
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
