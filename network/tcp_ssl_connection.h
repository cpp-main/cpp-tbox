#ifndef TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522
#define TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

#include "socket_fd.h"
#include "buffer.h"
#include "byte_stream.h"
#include "sockaddr.h"
#include "ssl_ctx.h"
#include "ssl.h"

namespace tbox {
namespace network {

class TcpSslConnection : public ByteStream {
    friend class TcpSslAcceptor;
    friend class TcpSslConnector;

  public:
    virtual ~TcpSslConnection();

    NONCOPYABLE(TcpSslConnection);
    IMMOVABLE(TcpSslConnection);

  public:
    using DisconnectedCallback = std::function<void ()>;
    void setDisconnectedCallback(const DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    bool disconnect();  //! 主动断开
    bool shutdown(int howto);

    SockAddr peerAddr() const { return peer_addr_; }
    SocketFd socketFd() const { return fd_; }

    //! 是否已经失效了
    bool isExpired() const;

    void* setContext(void *new_context);
    void* getContext() const { return wp_context_; }

  public:
    //! 实现ByteStream的接口
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void bind(ByteStream *receiver) override;
    virtual void unbind() override;
    virtual bool send(const void *data_ptr, size_t data_size) override;

  protected:
    void onFdReadable(short);
    void onFdWritable(short);
    void onSocketClosed();

    void doHandshake();

  private:
    explicit TcpSslConnection(event::Loop *wp_loop, SocketFd fd, SslCtx *wp_ssl_ctx,
                              const SockAddr &peer_addr, bool is_accept_state);

  private:
    event::Loop *wp_loop_;
    SocketFd     fd_;
    Ssl         *sp_ssl_;
    SockAddr     peer_addr_;

    event::FdEvent *sp_read_event_;
    event::FdEvent *sp_write_event_;

    bool is_ssl_done_ = false;

    Buffer read_buffer_;
    size_t recv_threshold_ = 0;

    ReceiveCallback      recv_cb_;
    DisconnectedCallback disconnected_cb_;
    ByteStream  *wp_stream_ = nullptr;

    void *wp_context_ = nullptr;
    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522
