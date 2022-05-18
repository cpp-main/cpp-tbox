#include "tcp_server.h"

#include <cassert>
#include <limits>

#include <tbox/base/log.h>
#include <tbox/base/cabinet.hpp>

#include "tcp_acceptor.h"
#include "tcp_connection.h"

namespace tbox {
namespace network {

using namespace std::placeholders;

using TcpConns = cabinet::Cabinet<TcpConnection>;

//! 私有数据
struct TcpServer::Data {
    event::Loop *wp_loop = nullptr;

    ConnectedCallback       connected_cb;
    DisconnectedCallback    disconnected_cb;
    ReceiveCallback         receive_cb;
    size_t                  receive_threshold = 0;

    TcpAcceptor *sp_acceptor = nullptr;
    TcpConns conns;     //!< TcpConnection 容器

    State state = State::kNone;
    int cb_level = 0;
};

TcpServer::TcpServer(event::Loop *wp_loop) :
    d_(new Data)
{
    assert(d_ != nullptr);

    d_->wp_loop = wp_loop;
    d_->sp_acceptor = new TcpAcceptor(wp_loop);
}

TcpServer::~TcpServer()
{
    assert(d_->cb_level == 0);

    cleanup();
    CHECK_DELETE_RESET_OBJ(d_->sp_acceptor);

    delete d_;
}

bool TcpServer::initialize(const SockAddr &bind_addr, int listen_backlog)
{
    if (d_->state != State::kNone)
        return false;

    if (d_->sp_acceptor->initialize(bind_addr, listen_backlog)) {
        d_->sp_acceptor->setNewConnectionCallback(std::bind(&TcpServer::onTcpConnected, this, _1));
        d_->state = State::kInited;
        return true;
    }

    return false;
}

void TcpServer::setConnectedCallback(const ConnectedCallback &cb)
{
    d_->connected_cb = cb;
}

void TcpServer::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    d_->disconnected_cb = cb;
}

void TcpServer::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    d_->receive_cb = cb;
    d_->receive_threshold = threshold;
}

bool TcpServer::start()
{
    if (d_->state != State::kInited)
        return false;

    if (d_->sp_acceptor->start()) {
        d_->state = State::kRunning;
        return true;
    }
    return false;
}

void TcpServer::stop()
{
    if (d_->state != State::kRunning)
        return;

    d_->conns.foreach(
        [](TcpConnection *conn) {
            conn->disconnect();
            delete conn;
        }
    );
    d_->conns.clear();

    d_->sp_acceptor->stop();
    d_->state = State::kInited;
}

void TcpServer::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    d_->sp_acceptor->cleanup();

    d_->connected_cb = nullptr;
    d_->disconnected_cb = nullptr;
    d_->receive_cb = nullptr;
    d_->receive_threshold = 0;

    d_->state = State::kNone;
}

bool TcpServer::send(const ConnToken &client, const void *data_ptr, size_t data_size)
{
    auto conn = d_->conns.at(client);
    if (conn != nullptr)
        return conn->send(data_ptr, data_size);
    return false;
}

bool TcpServer::disconnect(const ConnToken &client)
{
    auto conn = d_->conns.free(client);
    if (conn != nullptr) {
        conn->disconnect();
        d_->wp_loop->run([conn] { delete conn; });
        return true;
    }
    return false;
}

bool TcpServer::shutdown(const ConnToken &client, int howto)
{
    auto conn = d_->conns.free(client);
    if (conn != nullptr)
        return conn->shutdown(howto);
    return false;
}

bool TcpServer::isClientValid(const ConnToken &client) const
{
    return d_->conns.at(client) != nullptr;
}

SockAddr TcpServer::getClientAddress(const ConnToken &client) const
{
    auto conn = d_->conns.at(client);
    if (conn != nullptr)
        return conn->peerAddr();
    return SockAddr();
}

void* TcpServer::setContext(const ConnToken &client, void* context)
{
    auto conn = d_->conns.at(client);
    if (conn != nullptr)
        return conn->setContext(context);
    return nullptr;
}

void* TcpServer::getContext(const ConnToken &client) const
{
    auto conn = d_->conns.at(client);
    if (conn != nullptr)
        return conn->getContext();
    return nullptr;
}

TcpServer::State TcpServer::state() const
{
    return d_->state;
}

void TcpServer::onTcpConnected(TcpConnection *new_conn)
{
    ConnToken client = d_->conns.alloc(new_conn);
    new_conn->setReceiveCallback(std::bind(&TcpServer::onTcpReceived, this, client, _1), d_->receive_threshold);
    new_conn->setDisconnectedCallback(std::bind(&TcpServer::onTcpDisconnected, this, client));

    ++d_->cb_level;
    if (d_->connected_cb)
        d_->connected_cb(client);
    --d_->cb_level;
}

void TcpServer::onTcpDisconnected(const ConnToken &client)
{
    ++d_->cb_level;
    if (d_->disconnected_cb)
        d_->disconnected_cb(client);
    --d_->cb_level;

    TcpConnection *conn = d_->conns.free(client);
    d_->wp_loop->runNext([conn] { CHECK_DELETE_OBJ(conn); });
    //! 为什么先回调，再访问后面？是为了在回调中还能访问到TcpConnection对象
}

void TcpServer::onTcpReceived(const ConnToken &client, Buffer &buff)
{
    ++d_->cb_level;
    if (d_->receive_cb)
        d_->receive_cb(client, buff);
    --d_->cb_level;
}

}
}
