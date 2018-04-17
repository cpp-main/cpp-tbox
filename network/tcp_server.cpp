#include "tcp_server.h"

#include <cassert>
#include <limits>

#include <tbox/base/log.h>
#include "tcp_acceptor.h"
#include "tcp_connection.h"

namespace tbox {
namespace network {

using namespace std::placeholders;

TcpServer::TcpServer(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    sp_acceptor_(new TcpAcceptor(wp_loop))
{ }

TcpServer::~TcpServer()
{
    assert(cb_level_ == 0);

    cleanup();
    CHECK_DELETE_RESET_OBJ(sp_acceptor_);
}

bool TcpServer::initialize(const SockAddr &bind_addr, int listen_backlog)
{
    if (state_ != State::kNone)
        return false;

    if (sp_acceptor_->initialize(bind_addr, listen_backlog)) {
        sp_acceptor_->setNewConnectionCallback(std::bind(&TcpServer::onTcpConnected, this, _1));
        state_ = State::kInited;
        return true;
    }

    return false;
}

bool TcpServer::start()
{
    if (state_ != State::kInited)
        return false;

    if (sp_acceptor_->start()) {
        state_ = State::kRunning;
        return true;
    }
    return false;
}

bool TcpServer::stop()
{
    if (state_ != State::kRunning)
        return false;

    conns_.foreach(
        [](TcpConnection *conn) {
            conn->disconnect();
            delete conn;
        }
    );
    conns_.clear();

    if (sp_acceptor_->stop()) {
        state_ = State::kInited;
        return true;
    }
    return false;
}

void TcpServer::cleanup()
{
    stop();

    if (state_ != State::kInited)
        return;

    connected_cb_ = nullptr;
    disconnected_cb_ = nullptr;
    receive_cb_ = nullptr;
    receive_threshold_ = 0;

    state_ = State::kNone;
}

bool TcpServer::send(const Client &client, const void *data_ptr, size_t data_size)
{
    auto conn = conns_.at(client);
    if (conn != nullptr)
        return conn->send(data_ptr, data_size);
    return false;
}

bool TcpServer::disconnect(const Client &client)
{
    auto conn = conns_.at(client);
    if (conn != nullptr)
        return conn->disconnect();
    return false;
}

bool TcpServer::isClientValid(const Client &client) const
{
    return conns_.at(client) != nullptr;
}

SockAddr TcpServer::getClientAddress(const Client &client) const
{
    auto conn = conns_.at(client);
    if (conn != nullptr)
        return conn->peerAddr();
    return SockAddr();
}

void TcpServer::onTcpConnected(TcpConnection *new_conn)
{
    Client client = conns_.insert(new_conn);
    new_conn->setReceiveCallback(std::bind(&TcpServer::onTcpReceived, this, client, _1), receive_threshold_);
    new_conn->setDisconnectedCallback(std::bind(&TcpServer::onTcpDisconnected, this, client));

    ++cb_level_;
    if (connected_cb_)
        connected_cb_(client);
    --cb_level_;
}

void TcpServer::onTcpDisconnected(const Client &client)
{
    TcpConnection *conn = conns_.remove(client);
    wp_loop_->runNext([conn] { CHECK_DELETE_OBJ(conn); });

    ++cb_level_;
    if (disconnected_cb_)
        disconnected_cb_(client);
    --cb_level_;
}

void TcpServer::onTcpReceived(const Client &client, Buffer &buff)
{
    ++cb_level_;
    if (receive_cb_)
        receive_cb_(client, buff);
    --cb_level_;
}

}
}
