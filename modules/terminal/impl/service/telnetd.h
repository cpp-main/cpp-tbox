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
#ifndef TBOX_TERMINAL_TELNETD_IMPL_H_20220214
#define TBOX_TERMINAL_TELNETD_IMPL_H_20220214

#include <tbox/network/tcp_server.h>

#include <map>

#include "../../service/telnetd.h"
#include "../../connection.h"

namespace tbox {
namespace terminal {

using namespace event;
using namespace network;

class Telnetd::Impl : public Connection {
  public:
    Impl(Loop *wp_loop, TerminalInteract *wp_terminal);
    virtual ~Impl() override;

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  public:
    virtual bool send(const SessionToken &st, char ch) override;
    virtual bool send(const SessionToken &st, const std::string &str) override;
    virtual bool endSession(const SessionToken &st) override;
    virtual bool isValid(const SessionToken &st) const override;

  protected:
    enum Cmd : uint8_t {
        kEOF = 236,
        kSUSP,
        kABORT,
        kEOR,
        kSE,    //!< Sub end
        kNOP,
        kDM,    //!< Data Mark
        kBRK,   //!< Break
        kIP,    //!< Interrupt Process
        kAO,    //!< Abort output
        kAYT,   //!< Are you here
        kEC,    //!< Erase character
        kEL,    //!< Erase line
        kGA,    //!< Go ahead
        kSB,    //!< Sub begin
        kWILL, kWONT, kDO, kDONT,
        kIAC
    };

    enum Opt : uint8_t {
        kECHO = 1,
        kSGA = 3,
        kSTATUS = 5,
        kCLOCK = 6,
        kTYPE = 24,
        kWINDOW = 31,
        kSPEED = 32,
    };

    bool send(const TcpServer::ConnToken &ct, const void *data_ptr, size_t data_size);

    void sendString(const TcpServer::ConnToken &ct, const std::string &str);
    void sendNego(const TcpServer::ConnToken &ct, Cmd cmd, Opt opt);
    void sendCmd(const TcpServer::ConnToken &ct, Cmd cmd);
    void sendSub(const TcpServer::ConnToken &ct, Opt o, const uint8_t *p, size_t s);

    void onTcpConnected(const TcpServer::ConnToken &ct);
    void onTcpDisconnected(const TcpServer::ConnToken &ct);
    void onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff);

    void onRecvString(const TcpServer::ConnToken &ct, const std::string &str);
    void onRecvNego(const TcpServer::ConnToken &ct, Cmd cmd, Opt opt);
    void onRecvCmd(const TcpServer::ConnToken &ct, Cmd cmd);
    void onRecvSub(const TcpServer::ConnToken &ct, Opt opt, const uint8_t *p, size_t s);

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;

    std::map<SessionToken, TcpServer::ConnToken> session_to_client_;
    std::map<TcpServer::ConnToken, SessionToken> client_to_session_;
};

}
}

#endif //TBOX_TERMINAL_TELNETD_IMPL_H_20220214
