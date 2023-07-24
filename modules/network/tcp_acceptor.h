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
#ifndef TBOX_NETWORK_TCP_ACCEPTOR_20180114
#define TBOX_NETWORK_TCP_ACCEPTOR_20180114

#include <functional>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

#include "sockaddr.h"
#include "socket_fd.h"

namespace tbox {
namespace network {

class TcpConnection;

class TcpAcceptor {
  public:
    explicit TcpAcceptor(event::Loop *wp_loop);
    virtual ~TcpAcceptor();

    NONCOPYABLE(TcpAcceptor);
    IMMOVABLE(TcpAcceptor);

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);

    using NewConnectionCallback = std::function<void (TcpConnection*)>;
    void setNewConnectionCallback(const NewConnectionCallback &cb) { new_conn_cb_ = cb; }

    bool start();
    bool stop();

    void cleanup();

  protected:
    virtual SocketFd createSocket(SockAddr::Type addr_type);
    virtual int bindAddress(SocketFd sock_fd, const SockAddr &bind_addr);

    void onSocketRead(short events);    //! 处理新的连接请求
    void onClientConnected();

  private:
    event::Loop *wp_loop_ = nullptr;
    SockAddr bind_addr_;

    NewConnectionCallback new_conn_cb_;

    SocketFd sock_fd_;
    event::FdEvent *sp_read_ev_ = nullptr;

    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_ACCEPTOR_20180114
