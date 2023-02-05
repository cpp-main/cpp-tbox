#include "async_channel.h"
#include <cstring>
#include <algorithm>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace tbox {
namespace log {

AsyncChannel::AsyncChannel()
{ }

AsyncChannel::~AsyncChannel()
{ }

bool AsyncChannel::initialize(const Config &cfg)
{
    if (!async_pipe_.initialize(cfg))
        return false;

    using namespace std::placeholders;
    async_pipe_.setCallback(std::bind(&AsyncChannel::onLogBackEnd, this, _1, _2));

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

}
}
