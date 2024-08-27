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
#include "tcp_acceptor.h"

#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/util/fs.h>

#include "tcp_connection.h"

namespace tbox {
namespace network {

TcpAcceptor::TcpAcceptor(event::Loop *wp_loop) :
    wp_loop_(wp_loop)
{ }

TcpAcceptor::~TcpAcceptor()
{
    TBOX_ASSERT(cb_level_ == 0);
    if (sp_read_ev_ != nullptr)
        cleanup();
}

bool TcpAcceptor::initialize(const SockAddr &bind_addr, int listen_backlog)
{
    LogDbg("bind_addr:%s, backlog:%d", bind_addr.toString().c_str(), listen_backlog);

    bind_addr_ = bind_addr;

    SocketFd sock_fd = createSocket(bind_addr.type());
    if (sock_fd.isNull()) {
        LogErr("create socket fail");
        return false;
    }

    int bind_ret = bindAddress(sock_fd, bind_addr);
    if (bind_ret < 0) {
        LogErr("bind address %s fail", bind_addr.toString().c_str());
        return false;
    }

    int listen_ret = sock_fd.listen(listen_backlog);
    if (listen_ret < 0) {
        LogErr("listen fail");
        return false;
    }

    sock_fd_ = std::move(sock_fd);
    CHECK_DELETE_RESET_OBJ(sp_read_ev_);
    sp_read_ev_ = wp_loop_->newFdEvent("TcpAcceptor::sp_read_ev_");
    sp_read_ev_->initialize(sock_fd_.get(), event::FdEvent::kReadEvent, event::Event::Mode::kPersist);
    sp_read_ev_->setCallback(std::bind(&TcpAcceptor::onSocketRead, this, std::placeholders::_1));

    return true;
}

SocketFd TcpAcceptor::createSocket(SockAddr::Type addr_type)
{
    int flags = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
    if (addr_type == SockAddr::Type::kIPv4)
        return SocketFd::CreateSocket(AF_INET, flags, 0);
    else if (addr_type == SockAddr::Type::kLocal)
        return SocketFd::CreateSocket(AF_LOCAL, flags, 0);
    else {
        LogErr("socket type not support");
        return SocketFd();
    }
}

int TcpAcceptor::bindAddress(SocketFd sock_fd, const SockAddr &bind_addr)
{
    if (bind_addr.type() == SockAddr::Type::kIPv4) {
        sock_fd.setReuseAddress(true);

        struct sockaddr_in sock_addr;
        socklen_t len = bind_addr.toSockAddr(sock_addr);
        return sock_fd.bind((const struct sockaddr*)&sock_addr, len);

    } else if (bind_addr.type() == SockAddr::Type::kLocal) {
        //! 为防止存在的同名文件导致bind失败，在bind之前要先尝试删除原有的文件
        ::unlink(bind_addr_.toString().c_str());

        struct sockaddr_un sock_addr;
        socklen_t len = bind_addr.toSockAddr(sock_addr);
        return sock_fd.bind((const struct sockaddr*)&sock_addr, len);
    } else {
        LogErr("bind addr type not support");
        return -1;
    }
}

bool TcpAcceptor::start()
{
    if (sp_read_ev_ != nullptr)
        return sp_read_ev_->enable();
    return false;
}

bool TcpAcceptor::stop()
{
    if (sp_read_ev_ != nullptr)
        return sp_read_ev_->disable();
    return false;
}

void TcpAcceptor::cleanup()
{
    CHECK_DELETE_RESET_OBJ(sp_read_ev_);
    sock_fd_.close();

    //! 对于Unix Domain的Socket在退出的时候要删除对应的socket文件
    if (bind_addr_.type() == SockAddr::Type::kLocal) {
        auto socket_file = bind_addr_.toString();
        util::fs::RemoveFile(socket_file);
    }
}

void TcpAcceptor::onSocketRead(short events)
{
    if (events & event::FdEvent::kReadEvent)
        onClientConnected();
}

void TcpAcceptor::onClientConnected()
{
    RECORD_SCOPE();
    struct sockaddr addr;
    socklen_t addr_len = sizeof(addr);
    SocketFd peer_sock = sock_fd_.accept(&addr, &addr_len);
    if (peer_sock.isNull()) {
        LogNotice("accept fail");
        return;
    }

    SockAddr peer_addr(addr, addr_len);
    LogInfo("%s accepted new connection: %s", bind_addr_.toString().c_str(), peer_addr.toString().c_str());

    if (new_conn_cb_) {
        auto sp_connection = new TcpConnection(wp_loop_, peer_sock, peer_addr);
        sp_connection->enable();
        ++cb_level_;
        new_conn_cb_(sp_connection);
        --cb_level_;
    } else {
        LogWarn("%s need connect cb", bind_addr_.toString().c_str());
    }
}

}
}
