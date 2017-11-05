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

  public: //! socket相关的设置
    bool setSocketOpt(int level, int optname, int value);

    bool setReuseAddress(bool enable);  //! 设置可重用地址
    bool setBroadcast(bool enable);     //! 设置是否允许广播
    bool setKeepalive(bool enable);     //! 设置是否开启保活

    bool setRecvBufferSize(int size);   //! 设置接收缓冲大小
    bool setSendBufferSize(int size);   //! 设置发送缓冲大小

    bool setRecvLowWater(int size);     //! 设置接收低水位标记
    bool setSendLowWater(int size);     //! 设置发送低水位标记

  protected:
};

}
}

#endif //TBOX_NETWORK_SOCKET_FD_H_20171105
