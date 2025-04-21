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
#ifndef TBOX_HTTP_SERVER_H_20220502
#define TBOX_HTTP_SERVER_H_20220502

#include <vector>
#include <map>
#include <set>
#include <limits>
#include <tbox/network/tcp_server.h>

#include "server.h"
#include "request_parser.h"

namespace tbox {
namespace http {

struct Respond;

namespace server {

using namespace event;
using namespace network;
using namespace std;

class Server::Impl {
  public:
    Impl(Server *wp_parent, event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

    State state() const { return state_; }
    void setContextLogEnable(bool enable) { context_log_enable_ = enable; }

  public:
    void use(RequestHandler &&handler);
    void use(Middleware *wp_middleware);

    void commitRespond(const TcpServer::ConnToken &ct, int index, Respond *res);

  private:

    void onTcpConnected(const TcpServer::ConnToken &ct);
    void onTcpDisconnected(const TcpServer::ConnToken &ct);
    void onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff);
    void onTcpSendCompleted(const TcpServer::ConnToken &ct);

    //! 连接信息
    struct Connection {
        RequestParser req_parser;
        int req_index = 0;  //!< 下一个请求的index
        int res_index = 0;  //!< 下一个要求回复的index，用于实现按顺序回复
        int close_index = numeric_limits<int>::max();   //!< 需要关闭连接的index
        map<int, Respond*> res_buff;  //!< 暂存器

        ~Connection();
    };

    void handle(ContextSptr ctx, size_t cb_index);

  private:
    Server *wp_parent_;

    TcpServer tcp_server_;
    vector<RequestHandler> req_handler_;
    set<Connection*> conns_;    //! 仅用于保存Connection指针，用于释放
    State state_ = State::kNone;
    bool context_log_enable_ = false;

    int cb_level_ = 0;
};

}
}
}

#endif //endif //endif //TBOX_HTTP_SERVER_H_20220502
