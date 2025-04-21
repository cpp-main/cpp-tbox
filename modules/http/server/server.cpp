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
#include "server.h"
#include "server_imp.h"

namespace tbox {
namespace http {
namespace server {

Server::Server(Loop *wp_loop) :
    impl_(new Impl(this, wp_loop))
{ }

Server::~Server()
{
    delete impl_;
}

bool Server::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    return impl_->initialize(bind_addr, listen_backlog);
}

bool Server::start()
{
    return impl_->start();
}

void Server::stop()
{
    impl_->stop();
}

void Server::cleanup()
{
    impl_->cleanup();
}

Server::State Server::state() const
{
    return impl_->state();
}

void Server::setContextLogEnable(bool enable)
{
    return impl_->setContextLogEnable(enable);
}

void Server::use(RequestHandler &&handler)
{
    impl_->use(std::move(handler));
}

void Server::use(Middleware *wp_middleware)
{
    impl_->use(wp_middleware);
}

}
}
}
