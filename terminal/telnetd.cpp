#include "telnetd.h"
#include <iostream>
#include <map>
#include <tbox/network/tcp_server.h>
#include <tbox/util/string.h>
#include <tbox/base/log.h>

#include "interface.h"

namespace tbox::terminal {

using namespace std;
using namespace util;
using namespace event;
using namespace network;
using namespace std::placeholders;

class Telnetd::Impl : public Connection {
  public:
    Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal);
    virtual ~Impl();

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  public:
    bool send(const Session &session, const std::string &str) override;
    bool endSession(const Session &session) override;
    bool isValid(const Session &session) const override;

  protected:
    void onTcpConnected(const TcpServer::Client &client);
    void onTcpDisconnected(const TcpServer::Client &client);
    void onTcpReceived(const TcpServer::Client &client, Buffer &buff);

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;

    std::map<Session, TcpServer::Client> session_to_client_;
    std::map<TcpServer::Client, Session> client_to_session_;
};

Telnetd::Telnetd(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    impl_(new Impl(wp_loop, wp_terminal))
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

Telnetd::Impl::Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    wp_loop_(wp_loop),
    wp_terminal_(wp_terminal),
    sp_tcp_(new TcpServer(wp_loop))
{
    assert(wp_loop_ != nullptr);
    assert(wp_terminal_ != nullptr);
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

bool Telnetd::Impl::send(const Session &session, const std::string &str)
{
    auto client = session_to_client_.at(session);
    if (session.isNull())
        return false;

    sp_tcp_->send(client, str.c_str(), str.size());
    return true;
}

bool Telnetd::Impl::endSession(const Session &session)
{
    auto client = session_to_client_.at(session);
    if (client.isNull())
        return false;

    client_to_session_.erase(client);
    session_to_client_.erase(session);
    sp_tcp_->disconnect(client);
    return true;
}

bool Telnetd::Impl::isValid(const Session &session) const
{
    return session_to_client_.find(session) != session_to_client_.end();
}

void Telnetd::Impl::onTcpConnected(const TcpServer::Client &client)
{
    cout << "from " << client.id() << " connected" << endl;
    auto session = wp_terminal_->newSession(this);
    client_to_session_[client] = session;
    session_to_client_[session] = client;
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::Client &client)
{
    cout << "from " << client.id() << " disconnected" << endl;
    auto session = client_to_session_.at(client);
    client_to_session_.erase(client);
    session_to_client_.erase(session);
}

void Telnetd::Impl::onTcpReceived(const TcpServer::Client &client, Buffer &buff)
{
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << "from " << client.id() << " recv " << buff.readableSize() << ": " << hex_str << endl;

    auto session = client_to_session_.at(client);
    std::string str(reinterpret_cast<const char*>(buff.readableBegin()), buff.readableSize());
    wp_terminal_->input(session, str);

    buff.hasReadAll();
}

}
