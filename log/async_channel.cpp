#include "async_channel.h"
#include <cstring>
#include <algorithm>

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

void AsyncChannel::onLogFrontEnd(LogContent *content)
{
    const int buff_size = LOG_MAX_LEN;
    char buff[buff_size];

    size_t remain_size = buff_size;

#define WRITE_PTR (buff + (buff_size - remain_size))

    char timestamp[TIMESTAMP_STRING_SIZE]; //!  "20170513 23:45:07.000000"
    _GetCurrTimeString(content, timestamp);

    auto len = snprintf(WRITE_PTR, remain_size, "<%c> %s %ld %s ",
                       level_name[content->level], timestamp, content->thread_id, content->module_id);
    remain_size -= len;

    if (remain_size > 2 && content->func_name != nullptr) {
        len = snprintf(WRITE_PTR, remain_size, "%s() ", content->func_name);
        remain_size -= len;
    }

    if (remain_size > 2 && content->fmt != nullptr) {
        if (content->with_args) {
            va_list args;
            va_copy(args, content->args);    //! 同上，va_list 要被复制了使用
            len = vsnprintf(WRITE_PTR, remain_size, content->fmt, args);
            remain_size -= len;
        } else {
            auto len = strlen(content->fmt);
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
        len = snprintf(WRITE_PTR, remain_size, "-- %s:%d", content->file_name, content->line);
        remain_size -= len;
    }

    *WRITE_PTR = '\0';  //! 追加结束符
    remain_size -= 1;

#undef WRITE_PTR

    async_pipe_.append(buff, (buff_size - remain_size));
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
        } else {
            std::for_each(start_ptr, end_ptr,
                [=] (char c) {
                    string_buff_.push_back(c);
                }
            );
        }
    }
}

}
}
