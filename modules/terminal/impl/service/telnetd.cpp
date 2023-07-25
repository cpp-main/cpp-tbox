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
#include "telnetd.h"

#include <iostream>
#include <algorithm>

#include <tbox/util/string.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

#include "../../terminal_interact.h"

namespace tbox {
namespace terminal {

using namespace std;
using namespace std::placeholders;
using namespace util;

Telnetd::Impl::Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    wp_loop_(wp_loop),
    wp_terminal_(wp_terminal),
    sp_tcp_(new TcpServer(wp_loop))
{
    TBOX_ASSERT(wp_loop_ != nullptr);
    TBOX_ASSERT(wp_terminal_ != nullptr);
    TBOX_ASSERT(sp_tcp_ != nullptr);
}

Telnetd::Impl::~Impl()
{
    delete sp_tcp_;
}

bool Telnetd::Impl::initialize(const std::string &bind_addr_str)
{
    auto bind_addr = SockAddr::FromString(bind_addr_str);
    if (!sp_tcp_->initialize(bind_addr, 1))
        return false;

    sp_tcp_->setConnectedCallback(std::bind(&Impl::onTcpConnected, this, _1));
    sp_tcp_->setReceiveCallback(std::bind(&Impl::onTcpReceived, this, _1, _2), 1);
    sp_tcp_->setDisconnectedCallback(std::bind(&Impl::onTcpDisconnected, this, _1));
    return true;
}

bool Telnetd::Impl::start()
{
    return sp_tcp_->start();
}

void Telnetd::Impl::stop()
{
    sp_tcp_->stop();
}

void Telnetd::Impl::cleanup()
{
    sp_tcp_->cleanup();
}

bool Telnetd::Impl::send(const SessionToken &st, const std::string &str)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, str.c_str(), str.size());
    return true;
}

bool Telnetd::Impl::send(const SessionToken &st, char ch)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, &ch, 1);
    return true;
}

bool Telnetd::Impl::endSession(const SessionToken &st)
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
        "Telnetd::endSession"
    );

    return true;
}

bool Telnetd::Impl::isValid(const SessionToken &st) const
{
    return session_to_client_.find(st) != session_to_client_.end();
}

void Telnetd::Impl::onTcpConnected(const TcpServer::ConnToken &ct)
{
#if 0
    cout << ct.id() << " connected" << endl;
#endif
    auto st = wp_terminal_->newSession(this);
    client_to_session_[ct] = st;
    session_to_client_[st] = ct;

    sendNego(ct, kDONT, kECHO);
    sendNego(ct, kDO, kWINDOW);
    sendNego(ct, kDO, kSPEED);
    sendNego(ct, kWILL, kECHO);
    sendNego(ct, kWILL, kSGA);

    wp_terminal_->onBegin(st);
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::ConnToken &ct)
{
#if 0
    cout << ct.id() << " disconnected" << endl;
#endif
    auto st = client_to_session_.at(ct);
    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    wp_terminal_->deleteSession(st);
}

bool Telnetd::Impl::send(const TcpServer::ConnToken &ct, const void *data_ptr, size_t data_size)
{
#if 0
    auto hex_str = string::RawDataToHexStr(data_ptr, data_size);
    cout << ct.id() << " << send " << data_size << ": " << hex_str << endl;
#endif
    return sp_tcp_->send(ct, data_ptr, data_size);
}

void Telnetd::Impl::sendString(const TcpServer::ConnToken &ct, const std::string &str)
{
    send(ct, str.data(), str.size());
}

void Telnetd::Impl::sendNego(const TcpServer::ConnToken &ct, Cmd cmd, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, cmd, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendCmd(const TcpServer::ConnToken &ct, Cmd cmd)
{
    const uint8_t tmp[] = { Cmd::kIAC, cmd };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendSub(const TcpServer::ConnToken &ct, Opt o, const uint8_t *p, size_t s)
{
    size_t size = s + 5;
    uint8_t tmp[size];
    tmp[0] = Cmd::kIAC;
    tmp[1] = Cmd::kSB;
    tmp[2] = o;
    memcpy(tmp + 3, p, s);
    tmp[size-2] = Cmd::kIAC;
    tmp[size-1] = Cmd::kSE;

    send(ct, tmp, size);
}

void Telnetd::Impl::onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff)
{
#if 0
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << ct.id() << " >> recv " << buff.readableSize() << ": " << hex_str << endl;
#endif
    while (buff.readableSize() != 0) {
        auto begin = buff.readableBegin();
        auto end   = begin + buff.readableSize();
        auto iter  = std::find(begin, end, Cmd::kIAC);
        auto size = iter - begin;
        if (size > 0) {
            onRecvString(ct, std::string(reinterpret_cast<const char *>(begin), size));
        } else {
            if (buff.readableSize() < 2)
                return;

            //! start with IAC
            uint8_t cmd = begin[1];
            if (cmd == Cmd::kWILL || cmd == Cmd::kWONT || cmd == Cmd::kDO || cmd == Cmd::kDONT) {
                if (buff.readableSize() < 3)
                    return;
                onRecvNego(ct, static_cast<Cmd>(cmd), static_cast<Opt>(begin[2]));
                size = 3;

            } else if (cmd == Cmd::kSB) {
                if (buff.readableSize() < 6)
                    return;

                Opt opt = static_cast<Opt>(begin[2]);
                auto cmd_end_iac = std::find(begin + 4, end, Cmd::kIAC);
                if (cmd_end_iac == end)
                    return;
                if ((cmd_end_iac + 1) == end)
                    return;

                onRecvSub(ct, opt, begin + 3, (cmd_end_iac - begin - 3));
                size = cmd_end_iac - begin + 2;

            } else {
                onRecvCmd(ct, static_cast<Cmd>(cmd));
                size = 2;
            }
        }
        buff.hasRead(size);
    }
}

void Telnetd::Impl::onRecvString(const TcpServer::ConnToken &ct, const std::string &str)
{
    auto st = client_to_session_.at(ct);
    wp_terminal_->onRecvString(st, str);
}

void Telnetd::Impl::onRecvNego(const TcpServer::ConnToken &ct, Cmd cmd, Opt opt)
{
    LogTrace("cmd:%x, opt:%x", cmd, opt);
    auto st = client_to_session_.at(ct);

    if (cmd == Cmd::kDONT)
        sendNego(ct, Cmd::kWONT, opt);
    else if (cmd == Cmd::kDO) {
        if (opt == Opt::kECHO) {
            auto opts = wp_terminal_->getOptions(st);
            wp_terminal_->setOptions(st, opts | TerminalInteract::kEnableEcho);
        }
    }
}

void Telnetd::Impl::onRecvCmd(const TcpServer::ConnToken &ct, Cmd cmd)
{
    LogTrace("cmd:%x", cmd);

    if (cmd == Cmd::kNOP)
        sendCmd(ct, Cmd::kNOP);
}

void Telnetd::Impl::onRecvSub(const TcpServer::ConnToken &ct, Opt opt, const uint8_t *p, size_t s)
{
    LogTrace("opt:%x, data:%s", opt, util::string::RawDataToHexStr(p, s).c_str());
    auto st = client_to_session_.at(ct);
    if (opt == kWINDOW) {
        uint16_t w = p[0] << 8 | p[1];
        uint16_t h = p[2] << 8 | p[3];
        wp_terminal_->onRecvWindowSize(st, w, h);
    }
}

}
}
