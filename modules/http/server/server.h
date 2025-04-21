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
#ifndef TBOX_HTTP_SERVER_H_20220501
#define TBOX_HTTP_SERVER_H_20220501

#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"
#include "context.h"

#include "types.h"

namespace tbox {
namespace http {
namespace server {

class Middleware;

class Server {
    friend Context;

  public:
    explicit Server(event::Loop *wp_loop);
    virtual ~Server();

  public:
    bool initialize(const network::SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

    enum class State { kNone, kInited, kRunning };
    State state() const;
    void setContextLogEnable(bool enable);

  public:
    void use(RequestHandler &&handler);
    void use(Middleware *wp_middleware);

  private:
    class Impl;
    Impl *impl_;
};

}
}
}

#endif //TBOX_HTTP_SERVER_H_20220501
