#include "udp_socket.h"

#include <tbox/base/log.h>

namespace tbox {
namespace network {

UdpSocket::UdpSocket(bool enable_broadcast)
{
    socket_ = SocketFd::CreateUdpSocket();
    socket_.setBroadcast(enable_broadcast);
}

bool UdpSocket::bind(const SockAddr &addr)
{
    LogInfo("bind(%s)", addr.toString().c_str());

    struct sockaddr_storage s_addr;
    socklen_t s_len = addr.toSockAddr(s_addr);
    return socket_.bind((struct sockaddr*)&s_addr, s_len) == 0;
}

bool UdpSocket::connect(const SockAddr &addr)
{
    LogInfo("connect(%s)", addr.toString().c_str());

    struct sockaddr_storage s_addr;
    socklen_t s_len = addr.toSockAddr(s_addr);
    if (socket_.connect((struct sockaddr*)&s_addr, s_len) == 0) {
        connected_ = true;
        return true;
    }
    return false;
}

ssize_t UdpSocket::sendTo(const void *data_ptr, size_t data_size, const SockAddr &to_addr)
{
    struct sockaddr sock_addr;
    socklen_t len = to_addr.toSockAddr(sock_addr);
    return socket_.sendTo(data_ptr, data_size, 0, &sock_addr, len);
}

ssize_t UdpSocket::send(const void *data_ptr, size_t data_size)
{
    return socket_.send(data_ptr, data_size, 0);
}

}
}
