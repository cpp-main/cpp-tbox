#ifndef TBOX_NETWORK_TCP_SERVER_H_20180412
#define TBOX_NETWORK_TCP_SERVER_H_20180412

#include <tbox/base/defines.h>
#include <tbox/base/cabinet.hpp>
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
    using Client = cabinet::Token;

    enum class State {
        kNone,      //! 未初始化
        kInited,    //! 已初始化
        kRunning    //! 已启动
    };

    //! 设置绑定地址与backlog
    bool initialize(const SockAddr &bind_addr, int listen_backlog = 0);

    using ConnectedCallback     = std::function<void(const Client &)>;
    using DisconnectedCallback  = std::function<void(const Client &)>;
    using ReceiveCallback       = std::function<void(const Client &, Buffer &)>;

    //! 设置有新客户端连接时的回调
    void setConnectedCallback(const ConnectedCallback &cb);
    //! 设置有客户端断开时的回调
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    //! 设置接收到客户端消息时的回调
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold);

    bool start();   //!< 启动服务
    void stop();    //!< 停止服务，断开所有连接
    void cleanup(); //!< 清理

    //! 向指定客户端发送数据
    bool send(const Client &client, const void *data_ptr, size_t data_size);
    //! 断开指定客户端的连接
    bool disconnect(const Client &client);

    //! 检查客户端的连接是否有效
    bool isClientValid(const Client &client) const;
    //! 获取客户端的地址
    SockAddr getClientAddress(const Client &client) const;

    State state() const;

  protected:
    void onTcpConnected(TcpConnection *new_conn);
    void onTcpDisconnected(const Client &client);
    void onTcpReceived(const Client &client, Buffer &buff);

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_TCP_SERVER_H_20180412
