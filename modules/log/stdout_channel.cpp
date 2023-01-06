#include "stdout_channel.h"
#include <iostream>

namespace tbox {
namespace log {

void StdoutChannel::onLogFrontEnd(const void *data_ptr, size_t data_size)
{
    const char *str_ptr = static_cast<const char *>(data_ptr);
    std::cout << str_ptr << std::endl;
}

}
}
