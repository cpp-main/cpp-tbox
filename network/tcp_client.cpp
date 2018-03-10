#include "tcp_client.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include "tcp_connector.h"

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
    return false;   //TODO
}

bool TcpClient::stop()
{
    return false;
}

void TcpClient::cleanup()
{
    LogUndo();
}

void TcpClient::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    LogUndo();
}

bool TcpClient::send(const void *data_ptr, size_t data_size)
{
    LogUndo();
    return false;
}

void TcpClient::bind(ByteStream *receiver)
{
    LogUndo();
}

void TcpClient::unbind()
{
    LogUndo();
}

}
}
