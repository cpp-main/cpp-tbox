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
#ifndef TBOX_NETWORK_UDP_SOCKET_H_20171105
#define TBOX_NETWORK_UDP_SOCKET_H_20171105

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

#include "socket_fd.h"
#include "sockaddr.h"

namespace tbox {
namespace network {

class UdpSocket {
  public:
    explicit UdpSocket(bool enable_broadcast = false);  //! 用于只有发送的场景
    explicit UdpSocket(event::Loop *wp_loop, bool enable_broadcast = false);

    virtual ~UdpSocket();

    NONCOPYABLE(UdpSocket);

  public:
    bool bind(const SockAddr &addr);    //! 绑定地址与端口
    bool connect(const SockAddr &addr); //! 连接目标地址与端口
    //!NOTE:
    //!  bind() 与 connect() 不能一起使用，否则 bind() 会失效
    //!  为规避该问题，可以在 send() 函数中直接传入要发送的目标地址

    //! 设置接收回调
    using RecvCallback = std::function<void (const void *, size_t, const SockAddr &)>;
    void setRecvCallback(const RecvCallback &cb) { recv_cb_ = cb; }

    //! 发送数据
    ssize_t send(const void *data_ptr, size_t data_size, const SockAddr &to_addr);
    ssize_t send(const void *data_ptr, size_t data_size);   //! 要先 connect() 之后才能使用

    //! 开启与关闭接收功能
    bool enable();
    bool disable();

  protected:
    void onSocketEvent(short events);

  private:
    SocketFd socket_;
    bool connected_ = false;

    event::FdEvent  *sp_socket_ev_ = nullptr;
    int cb_level_ = 0;

    RecvCallback     recv_cb_;
};

}
}

#endif //TBOX_NETWORK_UDP_SOCKET_H_20171105
