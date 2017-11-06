#include "udp_socket.h"

namespace tbox {
namespace network {

UdpSocket::UdpSocket(bool enable_broadcast)
{
    socket_ = SocketFd::CreateUdpSocket();
    socket_.setBroadcast(enable_broadcast);
}

void UdpSocket::sendTo(const void *data_ptr, size_t data_size, const SockAddr &to_addr)
{
    struct sockaddr sock_addr;
    socklen_t len = to_addr.toSockAddr(sock_addr);
    socket_.sendTo(data_ptr, data_size, 0, &sock_addr, len);
}

}
}
