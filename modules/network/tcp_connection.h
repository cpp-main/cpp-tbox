/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
6 * ++ ----------.  \/\  .
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
#ifndef TBOX_NETWORK_TCP_CONNECTION_H_20180113
#define TBOX_NETWORK_TCP_CONNECTION_H_20180113

#include <tbox/event/loop.h>

#include "socket_fd.h"
#include "buffered_fd.h"
#include "byte_stream.h"
#include "sockaddr.h"

namespace tbox {
namespace network {

class TcpConnection : public ByteStream {
    friend class TcpAcceptor;
    friend class TcpConnector;
    friend class TcpFactory;

  public:
    virtual ~TcpConnection();

    NONCOPYABLE(TcpConnection);
    IMMOVABLE(TcpConnection);

  public:
    using DisconnectedCallback = std::function<void ()>;
    void setDisconnectedCallback(const DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    bool disconnect();
    bool shutdown(int howto);

    SockAddr peerAddr() const { return peer_addr_; }
    SocketFd socketFd() const;

    //! 是否已经失效了
    bool isExpired() const { return sp_buffered_fd_ == nullptr; }

    using ContextDeleter = std::function<void(void*)>;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const { return sp_context_; }

    //! 启用连接（开始 I/O 事件驱动）
    //! 由 TcpAcceptor/TcpConnector 在创建后调用
    void enable();

  public:
    //! 实现ByteStream的接口
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void setSendCompleteCallback(const SendCompleteCallback &cb) override;
    virtual void bind(ByteStream *receiver) override;
    virtual void unbind() override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual Buffer* getReceiveBuffer() override;

  protected:
    //! 基类构造函数，子类需在构造后自行创建 sp_buffered_fd_ 并调用 setupBufferedFd()
    explicit TcpConnection(event::Loop *wp_loop, const SockAddr &peer_addr);

    //! 初始化 BufferedFd 的回调（在子类创建 sp_buffered_fd_ 后调用）
    void setupBufferedFd();

    //! 断开连接的具体操作
    //! 子类可覆写此方法以在断开前执行额外操作（如 SSL_shutdown）
    virtual bool doDisconnect() = 0;

    //! 子类可覆写此方法以在 shutdown 时执行额外操作
    virtual bool doShutdown(int howto) = 0;

  protected:
    event::Loop *wp_loop_;
    BufferedFd  *sp_buffered_fd_ = nullptr;
    SockAddr    peer_addr_;

    DisconnectedCallback disconnected_cb_;
    void *sp_context_ = nullptr;
    ContextDeleter context_deleter_;

    int cb_level_ = 0;

  private:
    void onSocketClosed();
    void onReadError(int errnum);
};

}
}
#endif //TBOX_NETWORK_TCP_CONNECTION_H_20180113
