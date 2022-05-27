#ifndef TBOX_NETWORK_TCP_SSL_IMMATURE_CONNECTION_H_20220527
#define TBOX_NETWORK_TCP_SSL_IMMATURE_CONNECTION_H_20220527

#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

#include "socket_fd.h"
#include "sockaddr.h"
#include "ssl_ctx.h"
#include "ssl.h"

namespace tbox {
namespace network {

//! 没成成熟的连接，即SSL连接还没有完成的半连接
class TcpSslImmatureConnection {
    friend TcpSslConnection;

  public:
    explicit TcpSslImmatureConnection(event::Loop *wp_loop, SocketFd fd, SslCtx *wp_ssl_ctx,
                                      const SockAddr &peer_addr);
    virtual ~TcpSslImmatureConnection();

  public:
    using NoParamCallback = std::function<void ()>;
    void setDisconnectedCallback(const NoParamCallback &cb) { disconnected_cb_ = cb; }
    void setCompletedCallback(const NoParamCallback &cb) { completed_cb_ = cb; }

  private:
    event::Loop *wp_loop_;
    Ssl         *sp_ssl_;
    SocketFd     fd_;
    SockAddr     peer_addr_;

    event::FdEvent *sp_read_event_;
    event::FdEvent *sp_write_event_;

    NoParamCallback disconnected_cb_;
    NoParamCallback completed_cb_;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_SSL_IMMATURE_CONNECTION_H_20220527
