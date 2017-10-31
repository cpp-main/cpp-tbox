#include "buffered_fd.h"

#include <tbox/base/log.h>

namespace tbox {
namespace network {

BufferedFd::BufferedFd(event::Loop *wp_loop) :
    wp_loop_(wp_loop)
{ }

BufferedFd::~BufferedFd()
{
    LogUndo();
}

bool BufferedFd::initialize(int fd, short events)
{
    LogUndo();
    return false;
}

void BufferedFd::setReceiveCallback(const ReceiveCallback &func, size_t threshold)
{
    LogUndo();
}

bool BufferedFd::enable()
{
    LogUndo();
    return false;
}

bool BufferedFd::disable()
{
    LogUndo();
    return false;
}

bool BufferedFd::send(const void *p_data, size_t data_size)
{
    LogUndo();
    return false;
}

}
}
