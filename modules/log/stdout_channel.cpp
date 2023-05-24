#include "stdout_channel.h"
#include <iostream>

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

void StdoutChannel::writeLog(const char *str, size_t len)
{
    std::cout << str << std::endl;
}

}
}
