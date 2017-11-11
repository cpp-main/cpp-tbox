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

int SocketFd::connect(const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = ::connect(get(), addr, addrlen);
    if (ret < 0)
        LogErr("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

int SocketFd::bind(const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = ::bind(get(), addr, addrlen);
    if (ret < 0)
        LogErr("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::send(const void* data_ptr, size_t data_size, int flag)
{
    ssize_t ret = ::send(get(), data_ptr, data_size, flag);
    if (ret < 0)
        LogErr("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::sendTo(const void* data_ptr, size_t data_size, int flag, const sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t ret = ::sendto(get(), data_ptr, data_size, flag, dest_addr, addrlen);
    if (ret < 0)
        LogErr("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

bool SocketFd::setSocketOpt(int level, int optname, int value)
{
    if (::setsockopt(get(), level, optname, &value, sizeof(value)) != 0) {
        LogErr("fail, opt:%d, val:%d, errno:%d, %s",
               optname, value, errno, strerror(errno));
        return false;
    }
    return true;
}

bool SocketFd::setSocketOpt(int level, int optname, void *value, size_t size)
{
    if (::setsockopt(get(), level, optname, value, size) != 0) {
        LogErr("fail, opt:%d, errno:%d, %s",
               optname, errno, strerror(errno));
        return false;
    }
    return true;
}

bool SocketFd::setReuseAddress(bool enable)
{
    return setSocketOpt(SOL_SOCKET, SO_REUSEADDR, enable);
}

bool SocketFd::setBroadcast(bool enable)
{
    return setSocketOpt(SOL_SOCKET, SO_BROADCAST, enable);
}

bool SocketFd::setKeepalive(bool enable)
{
    return setSocketOpt(SOL_SOCKET, SO_KEEPALIVE, enable);
}

bool SocketFd::setRecvBufferSize(int size)
{
    return setSocketOpt(SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketFd::setSendBufferSize(int size)
{
    return setSocketOpt(SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketFd::setRecvLowWater(int size)
{
    return setSocketOpt(SOL_SOCKET, SO_RCVLOWAT, size);
}

bool SocketFd::setSendLowWater(int size)
{
    return setSocketOpt(SOL_SOCKET, SO_SNDLOWAT, size);
}

bool SocketFd::setLinger(bool enable, int linger)
{
    struct linger value = {
        .l_onoff = enable,
        .l_linger = linger
    };
    return setSocketOpt(SOL_SOCKET, SO_LINGER, &value, sizeof(value));
}

}
}
