#ifndef TBOX_NETWORK_TCP_SSL_CLIENT_H_20220522
#define TBOX_NETWORK_TCP_SSL_CLIENT_H_20220522

/**
 * 实现TcpSslClient类
 * 它封装TcpSslConnector与TcpSslConnection两个对象
 */

#include "byte_stream.h"
#include "sockaddr.h"

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace network {

class TcpSslConnector;
class TcpSslConnection;

class TcpSslClient : public ByteStream {
  public:
    explicit TcpSslClient(event::Loop *wp_loop);
    virtual ~TcpSslClient();

    NONCOPYABLE(TcpSslClient);
    IMMOVABLE(TcpSslClient);

    using ConnectedCallback    = std::function<void()>;
    using DisconnectedCallback = std::function<void()>;

    //!< 状态
    enum class State {
        kNone,          //!< 未初始化
        kInited,        //!< 已初始化
        kConnecting,    //!< 连接中
        kConnected      //!< 连接成功
    };

  public:
    bool initialize(const SockAddr &server_addr);
    void setConnectedCallback(const ConnectedCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    void setAutoReconnect(bool enable);

    bool start();   //!< 开始连接服务端
    void stop();    //!< 如果没有连接则成，则停止连接；否则断开连接

    bool shutdown(int howto);   //! 支持半关闭
    void cleanup();

    State state() const;

  public:   //! 实现ByteStream的接口
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual void bind(ByteStream *receiver) override;
    virtual void unbind() override;

  protected:
    void onTcpConnected(TcpSslConnection *new_conn);
    void onTcpDisconnected();

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}
#endif //TBOX_NETWORK_TCP_SSL_CLIENT_H_20220522