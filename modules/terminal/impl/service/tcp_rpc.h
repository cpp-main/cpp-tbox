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
#ifndef TBOX_TERMINAL_TCP_RPC_IMP_H_20220306
#define TBOX_TERMINAL_TCP_RPC_IMP_H_20220306

#include <map>

#include <tbox/network/tcp_server.h>

#include "../../connection.h"
#include "../../service/tcp_rpc.h"
#include "../../terminal_interact.h"

namespace tbox {
namespace terminal {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

class TcpRpc::Impl : Connection {
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
    bool send(const TcpServer::ConnToken &ct, const void *data_ptr, size_t data_size);

    void onTcpConnected(const TcpServer::ConnToken &ct);
    void onTcpDisconnected(const TcpServer::ConnToken &ct);
    void onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff);
    void onRecvString(const TcpServer::ConnToken &ct, const std::string &str);

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;

    map<SessionToken, TcpServer::ConnToken> session_to_client_;
    map<TcpServer::ConnToken, SessionToken> client_to_session_;
};

}
}

#endif //TBOX_TERMINAL_TCP_RPC_IMP_H_20220306
