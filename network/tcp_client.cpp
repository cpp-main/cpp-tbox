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
    CHECK_DELETE_RESET_OBJ(sp_connector_);
    LogUndo();  //TODO
}

bool TcpClient::initialize(const SockAddr &server_addr)
{
    using namespace std::placeholders;
    sp_connector_->initialize(server_addr);
    sp_connector_->setConnectedCallback(std::bind(&TcpClient::onTcpConnected, this, _1));

    LogUndo();  //TODO
    return false;
}

bool TcpClient::start()
{
    return sp_connector_->start();
}

bool TcpClient::stop()
{
    if (sp_connection_ != nullptr)
        sp_connection_->disconnect();

    sp_connector_->stop();
    return false;
}

void TcpClient::cleanup()
{
    LogUndo();
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

    wp_stream_receiver_ = receiver;
}

void TcpClient::unbind()
{
    if (sp_connection_ != nullptr)
        sp_connection_->unbind();

    wp_stream_receiver_ = nullptr;
}

void TcpClient::onTcpConnected(TcpConnection *new_conn)
{
    new_conn->setDisconnectedCallback(std::bind(&TcpClient::onTcpDisconnected, this));
    new_conn->setReceiveCallback(received_cb_, received_threshold_);
    if (wp_stream_receiver_ != nullptr)
        new_conn->bind(wp_stream_receiver_);

    CHECK_DELETE_RESET_OBJ(sp_connection_);
    sp_connection_ = new_conn;
}

void TcpClient::onTcpDisconnected()
{
    TcpConnection *tobe_delete = nullptr;
    std::swap(tobe_delete, sp_connection_);

    wp_loop_->runNext([tobe_delete] { delete tobe_delete; });

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

}
}
