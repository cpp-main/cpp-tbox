/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_NETWORK_TCP_SERVER_H_20180412
#define TBOX_NETWORK_TCP_SERVER_H_20180412

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>
#include <tbox/event/loop.h>
#include <tbox/util/buffer.h>

#include "sockaddr.h"

namespace tbox {
namespace network {

using Buffer = util::Buffer;

class TcpAcceptor;
class TcpConnection;

class TcpServer {
  public:
    explicit TcpServer(event::Loop *wp_loop);
    virtual ~TcpServer();

    NONCOPYABLE(TcpServer);
    IMMOVABLE(TcpServer);

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
    using SendCompleteCallback  = std::function<void(const ConnToken &)>;

    //! 设置有新客户端连接时的回调
    void setConnectedCallback(const ConnectedCallback &cb);
    //! 设置有客户端断开时的回调
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    //! 设置接收到客户端消息时的回调
    void setReceiveCallback(const ReceiveCallback &cb, size_t threshold);
    //! 设置数据发送完成回调
    void setSendCompleteCallback(const SendCompleteCallback &cb);

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
    using ContextDeleter = std::function<void(void*)>;
    void  setContext(const ConnToken &client, void *context, ContextDeleter &&deleter = nullptr);
    void* getContext(const ConnToken &client) const;

    Buffer* getClientReceiveBuffer(const ConnToken &client);

    State state() const;

  protected:
    void onTcpConnected(TcpConnection *new_conn);
    void onTcpDisconnected(const ConnToken &client);
    void onTcpReceived(const ConnToken &client, Buffer &buff);
    void onTcpSendCompleted(const ConnToken &client);

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_TCP_SERVER_H_20180412
