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
#include "context.h"
#include "server_imp.h"

namespace tbox {
namespace http {
namespace server {

struct Context::Data {
    Server::Impl *wp_server;
    cabinet::Token conn_token;
    int req_index;

    Request    *sp_req;
    Respond    *sp_res;
    /**
     * 重点说明一下 sp_req 与 sp_res 的生命期
     * sp_req 是由 Server::Impl 创建，通过构造函数传进来的，后面的生命期由 Context 负责
     * sp_res 是由 Context 在构造的时候创建的。在 Context::done() 时移交给 Server::Impl
     *        在移交之前，生命期由Context管，但移交之后便由Server::Impl管。
     * 一定要注意！
     */
};

Context::Context(Server *wp_server, const cabinet::Token &ct, int req_index, Request *req) :
    d_(new Data{ wp_server->impl_, ct, req_index, req, new Respond })
{
    d_->sp_res->status_code = StatusCode::k404_NotFound;
    d_->sp_res->http_ver = HttpVer::k1_1;
}

Context::~Context()
{
    d_->wp_server->commitRespond(d_->conn_token, d_->req_index, d_->sp_res);

    CHECK_DELETE_RESET_OBJ(d_->sp_req);
    CHECK_DELETE_RESET_OBJ(d_);
}

Request& Context::req() const
{
    return *(d_->sp_req);
}

Respond& Context::res() const
{
    return *(d_->sp_res);
}

}
}
}
