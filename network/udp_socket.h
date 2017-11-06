#ifndef TBOX_NETWORK_UDP_SOCKET_H_20171105
#define TBOX_NETWORK_UDP_SOCKET_H_20171105

#include <tbox/base/defines.h>

#include "socket_fd.h"
#include "sockaddr.h"

namespace tbox {
namespace network {

class UdpSocket {
  public:
    explicit UdpSocket(bool enable_broadcast = false);

    NONCOPYABLE(UdpSocket);

  public:
    void sendTo(const void *data_ptr, size_t data_size, const SockAddr &to_addr);

  private:
    SocketFd socket_;
};

}
}

#endif //TBOX_NETWORK_UDP_SOCKET_H_20171105
