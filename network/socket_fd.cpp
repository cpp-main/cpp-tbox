#include "socket_fd.h"

#include <errno.h>
#include <sys/socket.h>
#include <cstring>
#include <tbox/base/log.h>

namespace tbox {
namespace network {

SocketFd SocketFd::CreateSocket(int domain, int type, int protocal)
{
    int fd = ::socket(domain, type, protocal);
    if (fd < 0) {
        LogErr("create socket fail, errno:%d, %s", errno, strerror(errno));
        return SocketFd();
    }

    return SocketFd(fd);
}

SocketFd SocketFd::CreateUdpSocket()
{
    return CreateSocket(AF_INET, SOCK_DGRAM, 0);
}

SocketFd SocketFd::CreateTcpSocket()
{
    return CreateSocket(AF_INET, SOCK_STREAM, 0);
}

bool SocketFd::setReuseAddress(bool enable)
{
    int value = enable ? 1 : 0; 
    if (::setsockopt(get(), SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0) {
        LogErr("setsockopt fail, errno:%d, %s", errno, strerror(errno));
        return false;
    }
    return true;
}

bool SocketFd::setBroadcast(bool enable)
{
    int value = enable ? 1 : 0; 
    if (::setsockopt(get(), SOL_SOCKET, SO_BROADCAST, &value, sizeof(value)) != 0) {
        LogErr("setsockopt fail, errno:%d, %s", errno, strerror(errno));
        return false;
    }
    return true;
}

}
}
