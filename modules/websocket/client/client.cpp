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
namespace websocket {
namespace client {

Client::Client(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{ }

Client::~Client()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

bool Client::initialize(const network::SockAddr &server_addr, const std::string &url_path)
{
    return impl_->initialize(server_addr, url_path);
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

void Client::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void Client::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

void Client::setMessageCallback(const MessageCallback &cb)
{
    impl_->setMessageCallback(cb);
}

void Client::setErrorCallback(const ErrorCallback &cb)
{
    impl_->setErrorCallback(cb);
}

void Client::setAutoReconnect(bool enable)
{
    impl_->setAutoReconnect(enable);
}

void Client::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    impl_->setReconnectDelayCalcFunc(func);
}

bool Client::send(const std::string &text)
{
    return impl_->send(text);
}

bool Client::send(const void *data, size_t len)
{
    return impl_->send(data, len);
}

bool Client::sendBinary(const std::vector<uint8_t> &data)
{
    return impl_->sendBinary(data);
}

bool Client::close(uint16_t code, const std::string &reason)
{
    return impl_->close(code, reason);
}

bool Client::ping(const std::string &data)
{
    return impl_->ping(data);
}

bool Client::pong(const std::string &data)
{
    return impl_->pong(data);
}

bool Client::isExpired() const
{
    return impl_->isExpired();
}

network::SockAddr Client::peerAddr() const
{
    return impl_->peerAddr();
}

void Client::setContext(void *context, ContextDeleter &&deleter)
{
    impl_->setContext(context, std::move(deleter));
}

void* Client::getContext() const
{
    return impl_->getContext();
}

}
}
}
