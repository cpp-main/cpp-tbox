#ifndef TBOX_NETWORK_TCP_SSL_ACCEPTOR_20220522
#define TBOX_NETWORK_TCP_SSL_ACCEPTOR_20220522

#include <functional>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

#include "sockaddr.h"
#include "socket_fd.h"
#include "ssl_ctx.h"

namespace tbox {
namespace network {

class TcpSslConnection;

class TcpSslAcceptor {
  public:
    explicit TcpSslAcceptor(event::Loop *wp_loop);
    virtual ~TcpSslAcceptor();

    NONCOPYABLE(TcpSslAcceptor);
    IMMOVABLE(TcpSslAcceptor);

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);

    bool useCertificateFile(const std::string &filename, int filetype);
    bool usePrivateKeyFile(const std::string &filename, int filetype);
    bool checkPrivateKey();

    using NewConnectionCallback = std::function<void (TcpSslConnection*)>;
    void setNewConnectionCallback(const NewConnectionCallback &cb) { new_conn_cb_ = cb; }

    bool start();
    bool stop();

    void cleanup();

  protected:
    virtual SocketFd createSocket(SockAddr::Type addr_type);
    virtual int bindAddress(SocketFd sock_fd, const SockAddr &bind_addr);

    void onSocketRead(short events);    //! 处理新的连接请求
    void onClientConnected();
    void onClientSslFinished(TcpSslConnection *new_conn);

  private:
    event::Loop *wp_loop_ = nullptr;
    SslCtx  *sp_ssl_ctx_ = nullptr;
    SockAddr bind_addr_;

    uint8_t ssl_setting_bits = 0;

    NewConnectionCallback new_conn_cb_;

    SocketFd sock_fd_;
    event::FdEvent *sp_read_ev_ = nullptr;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_SSL_ACCEPTOR_20220522
