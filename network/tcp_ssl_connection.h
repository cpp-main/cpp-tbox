#ifndef TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522
#define TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522

#include <tbox/event/loop.h>

#include "socket_fd.h"
#include "buffered_fd.h"
#include "byte_stream.h"
#include "sockaddr.h"

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
    SocketFd socketFd() const;

    //! 是否已经失效了
    bool isExpired() const { return sp_buffered_fd_ == nullptr; }

    void* setContext(void *new_context);
    void* getContext() const { return wp_context_; }

  public:
    //! 实现ByteStream的接口
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void bind(ByteStream *receiver) override;
    virtual void unbind() override;
    virtual bool send(const void *data_ptr, size_t data_size) override;

  protected:
    void onSocketClosed();
    void onError(int errnum);

  private:
    explicit TcpSslConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr);
    void enable();

  private:
    event::Loop *wp_loop_;
    BufferedFd  *sp_buffered_fd_;
    SockAddr    peer_addr_;

    DisconnectedCallback disconnected_cb_;
    void *wp_context_ = nullptr;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_SSL_CONNECTION_H_20220522
