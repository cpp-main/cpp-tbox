#ifndef TBOX_NETWORK_SOCKET_FD_H_20171105
#define TBOX_NETWORK_SOCKET_FD_H_20171105

#include "fd.h"

namespace tbox {
namespace network {

//! socket 文件描述符
class SocketFd : public Fd {
  public:
    using Fd::Fd;
    using Fd::operator=;
    using Fd::swap;

  public:
    static SocketFd CreateSocket(int domain, int type, int protocal);
    static SocketFd CreateUdpSocket();
    static SocketFd CreateTcpSocket();

  public:
    //! socket相关的设置
    bool setReuseAddress(bool enable);
    bool setBroadcast(bool enable);
};

}
}

#endif //TBOX_NETWORK_SOCKET_FD_H_20171105
