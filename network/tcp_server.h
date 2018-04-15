#ifndef TBOX_NETWORK_TCP_SERVER_H_20180412
#define TBOX_NETWORK_TCP_SERVER_H_20180412

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

#include "sockaddr.h"
#include "buffer.h"

namespace tbox {
namespace network {

class TcpServer {
  public:
    explicit TcpServer(event::Loop *wp_loop);
    virtual ~TcpServer();

    NONCOPYABLE(TcpServer);
    IMMOVABLE(TcpServer);

  public:
    using ConnId = uint64_t;    //! 连接ID号

    //! 客户端ID
    struct ClientId {
        size_t pos = 0;
        ConnId id  = 0;

        bool operator == (const ClientId &rhs) const { return id == rhs.id; }
        bool operator <  (const ClientId &rhs) const { return id < rhs.id; }
    };

    //! 设置绑定地址与backlog
    bool initialize(const SockAddr &bind_addr, int listen_backlog = 0);

    using ConnectedCallback     = std::function<void(const ClientId &)>;
    using DisconnectedCallback  = std::function<void(const ClientId &)>;
    using ReceiveCallback       = std::function<void(const ClientId &, Buffer &)>;

    //! 设置有新客户端连接时的回调
    void setConnectedCallback(const ConnectedCallback &cb);
    //! 设置有客户端断开时的回调
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    //! 设置接收到客户端消息时的回调
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold);

    bool start();   //!< 启动服务
    bool stop();    //!< 停止服务
    void cleanup(); //!< 清理

    //! 向指定客户端发送数据
    bool send(const ClientId &client, const void *data_ptr, size_t data_size);
    //! 断开指定客户端的连接
    bool disconnect(const ClientId &client);

  private:
    ConnId last_conn_id_ = 0;
    //!TODO:
};

}
}

#endif //TBOX_NETWORK_TCP_SERVER_H_20180412
