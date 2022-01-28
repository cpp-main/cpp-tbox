#include "impl.h"
#include <cassert>
#include <iostream>
#include <tbox/base/log.h>
#include <tbox/util/string.h>

namespace tbox::telnetd {

using namespace std;
using namespace util;
using namespace std::placeholders;

Telnetd::Impl::Impl(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    sp_tcp_(new TcpServer(wp_loop))
{
    assert(wp_loop_ != nullptr);
    assert(sp_tcp_ != nullptr);
}

Telnetd::Impl::~Impl()
{
    delete sp_tcp_;
}

bool Telnetd::Impl::initialize(const std::string &bind_addr_str)
{
    auto bind_addr = SockAddr::FromString(bind_addr_str);
    if (!sp_tcp_->initialize(bind_addr))
        return false;

    sp_tcp_->setConnectedCallback(std::bind(&Impl::onTcpConnected, this, _1));
    sp_tcp_->setReceiveCallback(std::bind(&Impl::onTcpReceived, this, _1, _2), 0);
    sp_tcp_->setDisconnectedCallback(std::bind(&Impl::onTcpDisconnected, this, _1));
    return true;
}

bool Telnetd::Impl::start()
{
    return sp_tcp_->start();
}

void Telnetd::Impl::stop()
{
    sp_tcp_->stop();
}

void Telnetd::Impl::cleanup()
{
    sp_tcp_->cleanup();
}

void Telnetd::Impl::onTcpConnected(const TcpServer::Client &client)
{
    cout << "from " << client.id() << " connected" << endl;
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::Client &client)
{
    cout << "from " << client.id() << " disconnected" << endl;
}

void Telnetd::Impl::onTcpReceived(const TcpServer::Client &client, Buffer &buff)
{
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << "from " << client.id() << " recv " << buff.readableSize() << ": " << hex_str << endl;
    buff.hasReadAll();
}

}
