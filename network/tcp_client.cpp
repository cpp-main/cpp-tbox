#include "tcp_client.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>

#include "tcp_connector.h"
#include "tcp_connection.h"

namespace tbox {
namespace network {

TcpClient::TcpClient(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    sp_connector_(new TcpConnector(wp_loop))
{ }

TcpClient::~TcpClient()
{
    assert(cb_level_ == 0);

    if (state_ != State::kNone)
        cleanup();

    CHECK_DELETE_RESET_OBJ(sp_connector_);
}

bool TcpClient::initialize(const SockAddr &server_addr)
{
    if (state_ != State::kNone) {
        LogWarn("not in none state, cleanup first.");
        return false;
    }

    using namespace std::placeholders;
    sp_connector_->initialize(server_addr);
    sp_connector_->setConnectedCallback(std::bind(&TcpClient::onTcpConnected, this, _1));

    state_ = State::kIdle;
    return true;
}

bool TcpClient::start()
{
    if (state_ != State::kIdle) {
        LogWarn("not in idle state, initialize or stop first");
        return false;
    }

    state_ = State::kConnecting;
    return sp_connector_->start();
}

bool TcpClient::stop()
{
    if (state_ == State::kConnecting) {
        state_ = State::kIdle;
        return sp_connector_->stop();

    } else if (state_ == State::kConnected) {
        TcpConnection *tobe_delete = nullptr;
        std::swap(tobe_delete, sp_connection_);
        tobe_delete->disconnect();
        wp_loop_->runNext([tobe_delete] { delete tobe_delete; });
        state_ = State::kIdle;
        return true;

    } else {
        LogWarn("not in connecting or connected state, ignore");
        return false;
    }
}

void TcpClient::cleanup()
{
    if (state_ != State::kIdle)
        stop();

    connected_cb_ = nullptr;
    disconnected_cb_ = nullptr;
    received_cb_ = nullptr;
    received_threshold_ = 0;
    wp_receiver_ = nullptr;
    reconnect_enabled_ = true;

    state_ = State::kNone;
}

void TcpClient::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    if (sp_connection_ != nullptr)
        sp_connection_->setReceiveCallback(cb, threshold);

    received_cb_ = cb;
    received_threshold_ = threshold;
}

bool TcpClient::send(const void *data_ptr, size_t data_size)
{
    if (sp_connection_ != nullptr)
        return sp_connection_->send(data_ptr, data_size);

    return false;
}

void TcpClient::bind(ByteStream *receiver)
{
    if (sp_connection_ != nullptr)
        sp_connection_->bind(receiver);

    wp_receiver_ = receiver;
}

void TcpClient::unbind()
{
    if (sp_connection_ != nullptr)
        sp_connection_->unbind();

    wp_receiver_ = nullptr;
}

void TcpClient::onTcpConnected(TcpConnection *new_conn)
{
    new_conn->setDisconnectedCallback(std::bind(&TcpClient::onTcpDisconnected, this));
    new_conn->setReceiveCallback(received_cb_, received_threshold_);
    if (wp_receiver_ != nullptr)
        new_conn->bind(wp_receiver_);

    //! 可以直接释放，因为本函数一定是 sp_connector_ 对象调用的
    CHECK_DELETE_RESET_OBJ(sp_connection_);
    sp_connection_ = new_conn;

    state_ = State::kConnected;
}

void TcpClient::onTcpDisconnected()
{
    TcpConnection *tobe_delete = nullptr;
    std::swap(tobe_delete, sp_connection_);
    //! 这里要使用延后释放，因为本函数一定是 sp_connection_ 对象自己调用的
    wp_loop_->runNext([tobe_delete] { delete tobe_delete; });

    state_ = State::kIdle;

    if (reconnect_enabled_)
        start();

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

}
}
