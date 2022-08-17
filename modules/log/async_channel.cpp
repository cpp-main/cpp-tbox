#include "async_channel.h"
#include <cstring>
#include <algorithm>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

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

void AsyncChannel::onLogFrontEnd(const void *data_ptr, size_t data_size)
{
    async_pipe_.append(data_ptr, data_size);
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

}
}
