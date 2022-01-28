#include "telnetd.h"
#include <iostream>
#include <tbox/network/tcp_server.h>
#include <tbox/util/string.h>

namespace tbox::shell {

using namespace std;
using namespace util;
using namespace event;
using namespace network;
using namespace std::placeholders;

class Telnetd::Impl {
  public:
    Impl(event::Loop *wp_loop, ShellInteract *wp_shell);
    ~Impl();

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  protected:
    void onTcpConnected(const TcpServer::Client &client);
    void onTcpDisconnected(const TcpServer::Client &client);
    void onTcpReceived(const TcpServer::Client &client, Buffer &buff);

  private:
    Loop *wp_loop_ = nullptr;
    ShellInteract *wp_shell_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;
};

Telnetd::Telnetd(event::Loop *wp_loop, ShellInteract *wp_shell) :
    impl_(new Impl(wp_loop, wp_shell))
{
    assert(impl_ != nullptr);
}

Telnetd::~Telnetd()
{
    delete impl_;
}

bool Telnetd::initialize(const std::string &bind_addr)
{
    return impl_->initialize(bind_addr);
}

bool Telnetd::start()
{
    return impl_->start();
}

void Telnetd::stop()
{
    return impl_->stop();
}

void Telnetd::cleanup()
{
    return impl_->cleanup();
}

Telnetd::Impl::Impl(event::Loop *wp_loop, ShellInteract *wp_shell) :
    wp_loop_(wp_loop),
    wp_shell_(wp_shell),
    sp_tcp_(new TcpServer(wp_loop))
{
    assert(wp_loop_ != nullptr);
    assert(wp_shell_ != nullptr);
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
