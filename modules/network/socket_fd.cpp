/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "socket_fd.h"

#include <errno.h>
#include <sys/socket.h>
#include <cstring>
#include <tbox/base/log.h>

namespace tbox {
namespace network {

SocketFd::SocketFd() { }

SocketFd::SocketFd(int fd) : Fd(fd) { }

SocketFd::SocketFd(const Fd &fd) : Fd(fd) { }

SocketFd SocketFd::CreateSocket(int domain, int type, int protocal)
{
    int fd = ::socket(domain, type, protocal);
    if (fd < 0) {
        LogDbg("create socket fail, errno:%d, %s", errno, strerror(errno));
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
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

int SocketFd::bind(const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = ::bind(get(), addr, addrlen);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

int SocketFd::listen(int backlog)
{
    int ret = ::listen(get(), backlog);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

int SocketFd::accept(struct sockaddr *addr, socklen_t *addrlen)
{
    int ret = ::accept(get(), addr, addrlen);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::send(const void* data_ptr, size_t data_size, int flag)
{
    ssize_t ret = ::send(get(), data_ptr, data_size, flag);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::recv(void* data_ptr, size_t data_size, int flag)
{
    ssize_t ret = ::recv(get(), data_ptr, data_size, flag);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::sendTo(const void* data_ptr, size_t data_size, int flag, const sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t ret = ::sendto(get(), data_ptr, data_size, flag, dest_addr, addrlen);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

ssize_t SocketFd::recvFrom(void* data_ptr, size_t data_size, int flag, sockaddr *dest_addr, socklen_t *addrlen)
{
    ssize_t ret = ::recvfrom(get(), data_ptr, data_size, flag, dest_addr, addrlen);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

int SocketFd::shutdown(int howto)
{
    int ret = ::shutdown(get(), howto);
    if (ret < 0)
        LogDbg("fail, errno:%d, %s", errno, strerror(errno));
    return ret;
}

bool SocketFd::getSocketOpt(int level, int optname, void *optval, socklen_t *optlen)
{
    if (::getsockopt(get(), level, optname, optval, optlen) != 0) {
        LogDbg("fail, opt:%d, errno:%d, %s", optname, errno, strerror(errno));
        return false;
    }
    return true;
}

bool SocketFd::setSocketOpt(int level, int optname, int optval)
{
    if (::setsockopt(get(), level, optname, &optval, sizeof(optval)) != 0) {
        LogDbg("fail, opt:%d, val:%d, errno:%d, %s", optname, optval, errno, strerror(errno));
        return false;
    }
    return true;
}

bool SocketFd::setSocketOpt(int level, int optname, const void *optval, socklen_t optlen)
{
    if (::setsockopt(get(), level, optname, optval, optlen) != 0) {
        LogDbg("fail, opt:%d, errno:%d, %s", optname, errno, strerror(errno));
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
