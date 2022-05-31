#ifndef TBOX_NETWORK_TCP_SSL_SERVER_H_20220522
#define TBOX_NETWORK_TCP_SSL_SERVER_H_20220522

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>
#include <tbox/event/loop.h>

#include "sockaddr.h"
#include "buffer.h"

namespace tbox {
namespace network {

class TcpSslAcceptor;
class TcpSslConnection;

class TcpSslServer {
  public:
    explicit TcpSslServer(event::Loop *wp_loop);
    virtual ~TcpSslServer();

    NONCOPYABLE(TcpSslServer);
    IMMOVABLE(TcpSslServer);

  public:
    using ConnToken = cabinet::Token;

    enum class State {
        kNone,      //! 未初始化
        kInited,    //! 已初始化
        kRunning    //! 已启动
    };

    //! 设置绑定地址与backlog
    bool initialize(const SockAddr &bind_addr, int listen_backlog);

    using ConnectedCallback     = std::function<void(const ConnToken &)>;
    using DisconnectedCallback  = std::function<void(const ConnToken &)>;
    using ReceiveCallback       = std::function<void(const ConnToken &, Buffer &)>;

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
    bool send(const ConnToken &client, const void *data_ptr, size_t data_size);
    //! 断开指定客户端的连接
    bool disconnect(const ConnToken &client);
    //! 半关闭
    bool shutdown(const ConnToken &client, int howto);

    //! 检查客户端的连接是否有效
    bool isClientValid(const ConnToken &client) const;
    //! 获取客户端的地址
    SockAddr getClientAddress(const ConnToken &client) const;

    //! 设置上下文
    void* setContext(const ConnToken &client, void* context);
    void* getContext(const ConnToken &client) const;
    //! 注意：TcpSslServer只是保存该指针，不负责管理期生命期

    State state() const;

  protected:
    void onTcpConnected(TcpSslConnection *new_conn);
    void onTcpDisconnected(const ConnToken &client);
    void onTcpReceived(const ConnToken &client, Buffer &buff);

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_TCP_SSL_SERVER_H_20220522