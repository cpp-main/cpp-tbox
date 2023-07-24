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
#include "tcp_rpc.h"

#include <iostream>
#include <algorithm>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace terminal {

TcpRpc::Impl::Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    wp_loop_(wp_loop),
    wp_terminal_(wp_terminal),
    sp_tcp_(new TcpServer(wp_loop))
{
    TBOX_ASSERT(wp_loop_ != nullptr);
    TBOX_ASSERT(wp_terminal_ != nullptr);
    TBOX_ASSERT(sp_tcp_ != nullptr);
}

TcpRpc::Impl::~Impl()
{
    delete sp_tcp_;
}

bool TcpRpc::Impl::initialize(const std::string &bind_addr_str)
{
    auto bind_addr = SockAddr::FromString(bind_addr_str);
    if (!sp_tcp_->initialize(bind_addr, 1))
        return false;

    sp_tcp_->setConnectedCallback(std::bind(&Impl::onTcpConnected, this, _1));
    sp_tcp_->setReceiveCallback(std::bind(&Impl::onTcpReceived, this, _1, _2), 1);
    sp_tcp_->setDisconnectedCallback(std::bind(&Impl::onTcpDisconnected, this, _1));
    return true;
}

bool TcpRpc::Impl::start()
{
    return sp_tcp_->start();
}

void TcpRpc::Impl::stop()
{
    sp_tcp_->stop();
}

void TcpRpc::Impl::cleanup()
{
    sp_tcp_->cleanup();
}

bool TcpRpc::Impl::send(const SessionToken &st, const std::string &str)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, str.c_str(), str.size());
    return true;
}

bool TcpRpc::Impl::send(const SessionToken &st, char ch)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, &ch, 1);
    return true;
}

bool TcpRpc::Impl::endSession(const SessionToken &st)
{
    auto ct = session_to_client_.at(st);
    if (ct.isNull())
        return false;

    //! 委托执行，否则会出自我销毁的异常
    wp_loop_->runNext(
        [this, st, ct] {
            client_to_session_.erase(ct);
            session_to_client_.erase(st);
            sp_tcp_->disconnect(ct);
        },
        "TcpRpc::endSession"
    );

    return true;
}

bool TcpRpc::Impl::isValid(const SessionToken &st) const
{
    return session_to_client_.find(st) != session_to_client_.end();
}

void TcpRpc::Impl::onTcpConnected(const TcpServer::ConnToken &ct)
{
#if 0
    cout << ct.id() << " connected" << endl;
#endif

    auto st = wp_terminal_->newSession(this);
    client_to_session_[ct] = st;
    session_to_client_[st] = ct;

    wp_terminal_->setOptions(st, TerminalInteract::kQuietMode);
    wp_terminal_->onBegin(st);
}

void TcpRpc::Impl::onTcpDisconnected(const TcpServer::ConnToken &ct)
{
#if 0
    cout << ct.id() << " disconnected" << endl;
#endif

    auto st = client_to_session_.at(ct);
    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    wp_terminal_->deleteSession(st);
}

bool TcpRpc::Impl::send(const TcpServer::ConnToken &ct, const void *data_ptr, size_t data_size)
{
    return sp_tcp_->send(ct, data_ptr, data_size);
}

void TcpRpc::Impl::onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff)
{
    onRecvString(ct, std::string(reinterpret_cast<const char *>(buff.readableBegin()), buff.readableSize()));
    buff.hasReadAll();
}

void TcpRpc::Impl::onRecvString(const TcpServer::ConnToken &ct, const std::string &str)
{
    auto st = client_to_session_.at(ct);
    wp_terminal_->onRecvString(st, str);
}

}
}
