#include "stdout_channel.h"
#include <unistd.h>
#include <algorithm>

namespace tbox {
namespace log {

bool StdoutChannel::initialize()
{
    AsyncChannel::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    return AsyncChannel::initialize(cfg);
}

void StdoutChannel::appendLog(const char *str, size_t len)
{
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(buffer_);
    std::copy(str, str + len - 1, back_insert_iter);
}

void StdoutChannel::flushLog()
{
    ::write(STDOUT_FILENO, buffer_.data(), buffer_.size()); //! 写到终端

    buffer_.clear();
    if (buffer_.capacity() > 1024)
        buffer_.shrink_to_fit();
}

}
}
