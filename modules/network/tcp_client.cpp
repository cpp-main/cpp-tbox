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
#include "tcp_client.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include "tcp_connector.h"
#include "tcp_connection.h"

namespace tbox {
namespace network {

struct TcpClient::Data {
    event::Loop *wp_loop;
    State state = State::kNone;

    ConnectedCallback    connected_cb;
    DisconnectedCallback disconnected_cb;
    ReceiveCallback      received_cb;
    size_t               received_threshold = 0;
    SendCompleteCallback send_complete_cb;
    ByteStream          *wp_receiver = nullptr;
    bool reconnect_enabled = true;

    TcpConnector  *sp_connector  = nullptr;
    TcpConnection *sp_connection = nullptr;

    int cb_level = 0;
};

TcpClient::TcpClient(event::Loop *wp_loop) :
    d_(new Data)
{
    TBOX_ASSERT(d_ != nullptr);

    d_->wp_loop = wp_loop;
    d_->sp_connector = new TcpConnector(wp_loop);
}

TcpClient::~TcpClient()
{
    TBOX_ASSERT(d_->cb_level == 0);

    cleanup();

    CHECK_DELETE_RESET_OBJ(d_->sp_connection);
    CHECK_DELETE_RESET_OBJ(d_->sp_connector);

    delete d_;
}

bool TcpClient::initialize(const SockAddr &server_addr)
{
    if (d_->state != State::kNone) {
        LogWarn("not in none state, cleanup first.");
        return false;
    }

    using namespace std::placeholders;
    d_->sp_connector->initialize(server_addr);
    d_->sp_connector->setConnectedCallback(std::bind(&TcpClient::onTcpConnected, this, _1));

    d_->state = State::kInited;
    return true;
}

void TcpClient::setConnectedCallback(const ConnectedCallback &cb)
{
    d_->connected_cb = cb;
}

void TcpClient::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    d_->disconnected_cb = cb;
}

void TcpClient::setAutoReconnect(bool enable)
{
    d_->reconnect_enabled = enable;
}

bool TcpClient::start()
{
    if (d_->state != State::kInited) {
        LogWarn("not in idle state, initialize or stop first");
        return false;
    }

    d_->state = State::kConnecting;
    return d_->sp_connector->start();
}

void TcpClient::stop()
{
    if (d_->state == State::kConnecting) {
        d_->state = State::kInited;
        d_->sp_connector->stop();

    } else if (d_->state == State::kConnected) {
        TcpConnection *tobe_delete = nullptr;
        std::swap(tobe_delete, d_->sp_connection);
        tobe_delete->disconnect();

        d_->wp_loop->runNext(
            [tobe_delete] { delete tobe_delete; },
            "TcpClient::stop, delete"
        );

        d_->state = State::kInited;
    }
}

bool TcpClient::shutdown(int howto)
{
    if (d_->state == State::kConnected)
        return d_->sp_connection->shutdown(howto);
    return false;
}

void TcpClient::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    d_->sp_connector->cleanup();

    d_->connected_cb = nullptr;
    d_->disconnected_cb = nullptr;
    d_->received_cb = nullptr;
    d_->received_threshold = 0;
    d_->send_complete_cb = nullptr;
    d_->wp_receiver = nullptr;
    d_->reconnect_enabled = true;

    d_->state = State::kNone;
}

TcpClient::State TcpClient::state() const
{
    return d_->state;
}

void TcpClient::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->setReceiveCallback(cb, threshold);

    d_->received_cb = cb;
    d_->received_threshold = threshold;
}

void TcpClient::setSendCompleteCallback(const SendCompleteCallback &cb)
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->setSendCompleteCallback(cb);

    d_->send_complete_cb = cb;
}

bool TcpClient::send(const void *data_ptr, size_t data_size)
{
    if (d_->sp_connection != nullptr)
        return d_->sp_connection->send(data_ptr, data_size);

    return false;
}

void TcpClient::bind(ByteStream *receiver)
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->bind(receiver);

    d_->wp_receiver = receiver;
}

void TcpClient::unbind()
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->unbind();

    d_->wp_receiver = nullptr;
}

Buffer* TcpClient::getReceiveBuffer()
{
    if (d_->sp_connection != nullptr)
        return d_->sp_connection->getReceiveBuffer();
    return nullptr;
}

void TcpClient::onTcpConnected(TcpConnection *new_conn)
{
    RECORD_SCOPE();
    new_conn->setDisconnectedCallback(std::bind(&TcpClient::onTcpDisconnected, this));
    new_conn->setReceiveCallback(d_->received_cb, d_->received_threshold);
    new_conn->setSendCompleteCallback(d_->send_complete_cb);
    if (d_->wp_receiver != nullptr)
        new_conn->bind(d_->wp_receiver);

    //! 可以直接释放，因为本函数一定是 d_->sp_connector 对象调用的
    CHECK_DELETE_RESET_OBJ(d_->sp_connection);
    d_->sp_connection = new_conn;

    d_->state = State::kConnected;

    if (d_->connected_cb) {
        RECORD_SCOPE();
        ++d_->cb_level;
        d_->connected_cb();
        --d_->cb_level;
    }
}

void TcpClient::onTcpDisconnected()
{
    RECORD_SCOPE();
    TcpConnection *tobe_delete = nullptr;
    std::swap(tobe_delete, d_->sp_connection);

    //! 这里要使用延后释放，因为本函数一定是 d_->sp_connection 对象自己调用的
    d_->wp_loop->runNext(
        [tobe_delete] { delete tobe_delete; },
        "TcpClient::onTcpDisconnected, delete"
    );

    d_->state = State::kInited;

    if (d_->reconnect_enabled)
        start();

    if (d_->disconnected_cb) {
        RECORD_SCOPE();
        ++d_->cb_level;
        d_->disconnected_cb();
        --d_->cb_level;
    }
}

}
}
