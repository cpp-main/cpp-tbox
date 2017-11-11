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
    bool bind(const SockAddr &addr);    //! 绑定地址与端口
    bool connect(const SockAddr &addr); //! 连接目标地址与端口

    //! 无连接
    using RecvFromCallback = std::function<void (const void *, size_t, const SockAddr &)>;
    void setRecvCallback(const RecvFromCallback &cb) { recv_from_cb_ = cb; }
    ssize_t sendTo(const void *data_ptr, size_t data_size, const SockAddr &to_addr);

    //! 有连接
    using RecvCallback = std::function<void (const void *, size_t)>;
    void setRecvCallback(const RecvCallback &cb) { recv_cb_ = cb; }
    ssize_t send(const void *data_ptr, size_t data_size);

    using ErrorCallback = std::function<void ()>;
    void setErrorCallback(const ErrorCallback &cb) { error_cb_ = cb; }

  private:
    SocketFd socket_;
    bool connected_ = false;

    RecvFromCallback recv_from_cb_;
    RecvCallback     recv_cb_;
    ErrorCallback    error_cb_;
};

}
}

#endif //TBOX_NETWORK_UDP_SOCKET_H_20171105