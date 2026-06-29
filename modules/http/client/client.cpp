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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "client.h"
#include "client_impl.h"

namespace tbox {
namespace http {
namespace client {

Client::Client(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{ }

Client::~Client()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

bool Client::initialize(const network::SockAddr &server_addr)
{
    return impl_->initialize(server_addr);
}

void Client::setTlsConfig(const network::TlsConfig &config)
{
    impl_->setTlsConfig(config);
}

bool Client::start()
{
    return impl_->start();
}

void Client::stop()
{
    impl_->stop();
}

void Client::cleanup()
{
    impl_->cleanup();
}

Client::State Client::state() const
{
    return impl_->state();
}

void Client::request(const Request &req, const RespondCallback &cb)
{
    impl_->request(req, cb);
}

void Client::request(Method method, const std::string &path, const RespondCallback &cb)
{
    impl_->request(method, path, cb);
}

void Client::request(Method method, const std::string &path,
                     const std::string &body, const Headers &headers,
                     const RespondCallback &cb)
{
    impl_->request(method, path, body, headers, cb);
}

void Client::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void Client::setConnectFailCallback(const ConnectFailCallback &cb)
{
    impl_->setConnectFailCallback(cb);
}

void Client::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

void Client::setAutoReconnect(bool enable)
{
    impl_->setAutoReconnect(enable);
}

void Client::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    impl_->setReconnectDelayCalcFunc(func);
}

void Client::setRequestTimeout(std::chrono::milliseconds ms)
{
    impl_->setRequestTimeout(ms);
}

void Client::setContextLogEnable(bool enable)
{
    impl_->setContextLogEnable(enable);
}

}
}
}
