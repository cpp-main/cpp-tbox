#ifndef TBOX_NETWORK_TCP_SERVER_H_20180412
#define TBOX_NETWORK_TCP_SERVER_H_20180412

#include <vector>

#include <tbox/base/defines.h>
#include <tbox/base/object_container.hpp>
#include <tbox/event/loop.h>

#include "sockaddr.h"
#include "buffer.h"

namespace tbox {
namespace network {

class TcpAcceptor;
class TcpConnection;

class TcpServer {
  public:
    explicit TcpServer(event::Loop *wp_loop);
    virtual ~TcpServer();

    NONCOPYABLE(TcpServer);
    IMMOVABLE(TcpServer);

  public:
    using ConnContainer = ObjectContainer<TcpConnection>;
    using Client = ConnContainer::Token;

    //! 设置绑定地址与backlog
    bool initialize(const SockAddr &bind_addr, int listen_backlog = 0);

    using ConnectedCallback     = std::function<void(const Client &)>;
    using DisconnectedCallback  = std::function<void(const Client &)>;
    using ReceiveCallback       = std::function<void(const Client &, Buffer &)>;

    //! 设置有新客户端连接时的回调
    void setConnectedCallback(const ConnectedCallback &cb) { connected_cb_ = cb; }
    //! 设置有客户端断开时的回调
    void setDisconnectedCallback(const DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    //! 设置接收到客户端消息时的回调
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
    { receive_cb_ = cb; receive_threshold_ = threshold; }

    bool start();   //!< 启动服务
    bool stop();    //!< 停止服务，断开所有连接
    void cleanup(); //!< 清理

    //! 向指定客户端发送数据
    bool send(const Client &client, const void *data_ptr, size_t data_size);
    //! 断开指定客户端的连接
    bool disconnect(const Client &client);

    //! 检查客户端的连接是否有效
    bool isClientValid(const Client &client) const;
    //! 获取客户端的地址
    SockAddr getClientAddress(const Client &client) const;

  private:
    event::Loop *wp_loop_ = nullptr;

    ConnectedCallback       connected_cb_;
    DisconnectedCallback    disconnected_cb_;
    ReceiveCallback         receive_cb_;
    size_t                  receive_threshold_ = 0;

    TcpAcceptor *sp_acceptor_ = nullptr;
    ConnContainer conns_;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_SERVER_H_20180412
