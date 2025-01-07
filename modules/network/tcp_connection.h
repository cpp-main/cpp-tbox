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

  public:
    virtual ~TcpConnection();

    NONCOPYABLE(TcpConnection);
    IMMOVABLE(TcpConnection);

  public:
    using DisconnectedCallback = std::function<void ()>;
    void setDisconnectedCallback(const DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    bool disconnect();  //! 主动断开
    bool shutdown(int howto);

    SockAddr peerAddr() const { return peer_addr_; }
    SocketFd socketFd() const;

    //! 是否已经失效了
    bool isExpired() const { return sp_buffered_fd_ == nullptr; }

    using ContextDeleter = std::function<void(void*)>;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const { return sp_context_; }

  public:
    //! 实现ByteStream的接口
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void setSendCompleteCallback(const SendCompleteCallback &cb) override;
    virtual void bind(ByteStream *receiver) override;
    virtual void unbind() override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual Buffer* getReceiveBuffer() override;

  protected:
    void onSocketClosed();
    void onReadError(int errnum);

  private:
    explicit TcpConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr);
    void enable();

  private:
    event::Loop *wp_loop_;
    BufferedFd  *sp_buffered_fd_;
    SockAddr    peer_addr_;

    DisconnectedCallback disconnected_cb_;
    void *sp_context_ = nullptr;
    ContextDeleter context_deleter_;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_CONNECTION_H_20180113
