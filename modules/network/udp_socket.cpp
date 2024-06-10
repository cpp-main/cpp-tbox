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
#include "udp_socket.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <errno.h>

#define RECV_BUFF_SIZE  4096

namespace tbox {
namespace network {

using namespace std::placeholders;

UdpSocket::UdpSocket(bool enable_broadcast)
{
    socket_ = SocketFd::CreateUdpSocket();
    socket_.setBroadcast(enable_broadcast);
}

UdpSocket::UdpSocket(event::Loop *wp_loop, bool enable_broadcast)
{
    socket_ = SocketFd::CreateUdpSocket();
    socket_.setBroadcast(enable_broadcast);

    sp_socket_ev_ = wp_loop->newFdEvent("UdpSocket::sp_socket_ev_");
    sp_socket_ev_->initialize(socket_.get(), event::FdEvent::kReadEvent, event::Event::Mode::kPersist);
    sp_socket_ev_->setCallback(std::bind(&UdpSocket::onSocketEvent, this, _1));
}

UdpSocket::~UdpSocket()
{
    TBOX_ASSERT(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_socket_ev_);
}

bool UdpSocket::bind(const SockAddr &addr)
{
    LogInfo("bind(%s)", addr.toString().c_str());

    struct sockaddr_storage s_addr;
    socklen_t s_len = addr.toSockAddr(s_addr);
    return socket_.bind((struct sockaddr*)&s_addr, s_len) == 0;
}

bool UdpSocket::connect(const SockAddr &addr)
{
    LogInfo("connect(%s)", addr.toString().c_str());

    struct sockaddr_storage s_addr;
    socklen_t s_len = addr.toSockAddr(s_addr);
    if (socket_.connect((struct sockaddr*)&s_addr, s_len) == 0) {
        connected_ = true;
        return true;
    }
    return false;
}

ssize_t UdpSocket::send(const void *data_ptr, size_t data_size, const SockAddr &to_addr)
{
    struct sockaddr sock_addr;
    socklen_t len = to_addr.toSockAddr(sock_addr);
    return socket_.sendTo(data_ptr, data_size, 0, &sock_addr, len);
}

ssize_t UdpSocket::send(const void *data_ptr, size_t data_size)
{
    if (connected_) {
        return socket_.send(data_ptr, data_size, 0);
    } else {
        LogWarn("connect first");
        return 0;
    }
}

bool UdpSocket::enable()
{
    if (sp_socket_ev_ != nullptr)
        return sp_socket_ev_->enable();
    return false;
}

bool UdpSocket::disable()
{
    if (sp_socket_ev_ != nullptr)
        return sp_socket_ev_->disable();
    return false;
}

void UdpSocket::onSocketEvent(short events)
{
    if ((events & event::FdEvent::kReadEvent) == 0)
        return;

    RECORD_SCOPE();
    uint8_t read_buff[RECV_BUFF_SIZE];
    struct sockaddr_in peer_addr;
    socklen_t addr_size = sizeof(peer_addr);
    ssize_t rsize = socket_.recvFrom(read_buff, RECV_BUFF_SIZE, 0, (struct sockaddr*)&peer_addr, &addr_size);
    if (rsize > 0) {
        if (recv_cb_) {
            ++cb_level_;
            recv_cb_(read_buff, rsize, SockAddr(peer_addr));
            --cb_level_;
        } else
            LogWarn("recv_cb_ is null");

    } else if (rsize < 0) {
        LogWarn("errno: %d, %s", errno, strerror(errno));
    }
}

}
}
