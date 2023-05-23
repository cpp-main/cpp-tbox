#include "stdout_channel.h"
#include <iostream>

namespace tbox {
namespace log {

void StdoutChannel::writeLog(const char *str, size_t len)
{
    std::cout << str << std::endl;
}

}
}
