#ifndef TBOX_NETWORK_TCP_CLIENT_H_20180310
#define TBOX_NETWORK_TCP_CLIENT_H_20180310

/**
 * 实现TcpClient类
 * 它封装TcpConnector与TcpConnection两个对象
 */

#include "byte_stream.h"
#include "sockaddr.h"
#include <tbox/event/loop.h>

namespace tbox {
namespace network {

class TcpConnector;
class TcpConnection;

class TcpClient : public ByteStream {
  public:
    explicit TcpClient(event::Loop *wp_loop);
    virtual ~TcpClient();

    using ConnectedCallback    = std::function<void()>;
    using DisconnectedCallback = std::function<void()>;

  public:
    bool initialize(const SockAddr &server_addr);
    void setConnectedCallback(const ConnectedCallback &cb) { connected_cb_ = cb; }
    void setDisconnectedCallback(const DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    void setAutoReconnect(bool enable) { reconnect_enabled_ = enable; }

    bool start();
    bool stop();
    void cleanup();

    bool isConnected() const { return sp_connection_ == nullptr; }
    bool disconnect();

  public:   //! 实现ByteStream的接口
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    bool send(const void *data_ptr, size_t data_size) override;
    void bind(ByteStream *receiver) override;
    void unbind() override;

  protected:
    void onTcpConnected(TcpConnection *new_conn);
    void onTcpDisconnected();

  private:
    event::Loop *wp_loop_;

    ConnectedCallback    connected_cb_;
    DisconnectedCallback disconnected_cb_;
    ReceiveCallback      received_cb_;
    size_t               received_threshold_ = 0;
    ByteStream *wp_stream_receiver_ = nullptr;
    bool reconnect_enabled_ = true;

    TcpConnector  *sp_connector_  = nullptr;
    TcpConnection *sp_connection_ = nullptr;

    int cb_level_ = 0;
};

}
}
#endif //TBOX_NETWORK_TCP_CLIENT_H_20180310
