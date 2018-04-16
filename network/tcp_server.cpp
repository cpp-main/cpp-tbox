#include "tcp_server.h"

#include <cassert>
#include <limits>

#include <tbox/base/log.h>
#include "tcp_acceptor.h"
#include "tcp_connection.h"

namespace tbox {
namespace network {

TcpServer::TcpServer(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    sp_acceptor_(new TcpAcceptor(wp_loop))
{ }

TcpServer::~TcpServer()
{
    assert(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_acceptor_);
}

bool TcpServer::initialize(const SockAddr &bind_addr, int listen_backlog)
{
    LogUndo();
    return false;
}

bool TcpServer::start()
{
    return sp_acceptor_->start();
}

bool TcpServer::stop()
{
    LogUndo();
    return false;
}

void TcpServer::cleanup()
{
    LogUndo();
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
    LogUndo();
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


}
}
