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
    bool send(const SessionToken &session, const std::string &str) override;
    bool endSession(const SessionToken &session) override;
    bool isValid(const SessionToken &session) const override;

  protected:
    void onTcpConnected(const TcpServer::ClientToken &client);
    void onTcpDisconnected(const TcpServer::ClientToken &client);
    void onTcpReceived(const TcpServer::ClientToken &client, Buffer &buff);

    enum Cmd {
        kEOF = 236, kSUSP, kABORT, kEOR, kSE, kNOP, kDM,
        kBRK, kIP, kAO, kAYT, kEC, kEL, kGA, kSB,
        kWILL, kWONT, kDO, kDONT,
        kIAC
    };

    enum Opt {
        kECHO = 1,
        kSTATUS = 5,
        kCLOCK = 6,
        kTYPE = 24,
        kWINDOW = 31,
        kSPEED = 32,
    };

    void send(const TcpServer::ClientToken &token, Cmd c, Opt o);
    void send(const TcpServer::ClientToken &token, Opt o, const uint8_t *p, size_t s);

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;

    std::map<SessionToken, TcpServer::ClientToken> session_to_client_;
    std::map<TcpServer::ClientToken, SessionToken> client_to_session_;
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

bool Telnetd::Impl::send(const SessionToken &session, const std::string &str)
{
    auto client = session_to_client_.at(session);
    if (session.isNull())
        return false;

    sp_tcp_->send(client, str.c_str(), str.size());
    return true;
}

bool Telnetd::Impl::endSession(const SessionToken &session)
{
    auto client = session_to_client_.at(session);
    if (client.isNull())
        return false;

    client_to_session_.erase(client);
    session_to_client_.erase(session);
    sp_tcp_->disconnect(client);
    return true;
}

bool Telnetd::Impl::isValid(const SessionToken &session) const
{
    return session_to_client_.find(session) != session_to_client_.end();
}

void Telnetd::Impl::onTcpConnected(const TcpServer::ClientToken &client)
{
    cout << "from " << client.id() << " connected" << endl;

    auto session = wp_terminal_->newSession(this);
    client_to_session_[client] = session;
    session_to_client_[session] = client;
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::ClientToken &client)
{
    cout << "from " << client.id() << " disconnected" << endl;

    auto session = client_to_session_.at(client);
    client_to_session_.erase(client);
    session_to_client_.erase(session);
    wp_terminal_->deleteSession(session);
}

void Telnetd::Impl::onTcpReceived(const TcpServer::ClientToken &client, Buffer &buff)
{
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << "from " << client.id() << " recv " << buff.readableSize() << ": " << hex_str << endl;

    auto session = client_to_session_.at(client);
    std::string str(reinterpret_cast<const char*>(buff.readableBegin()), buff.readableSize());
    wp_terminal_->input(session, str);

    buff.hasReadAll();
}

void Telnetd::Impl::send(const TcpServer::ClientToken &t, Cmd c, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, c, o };
    sp_tcp_->send(t, tmp, sizeof(tmp));
}

void Telnetd::Impl::send(const TcpServer::ClientToken &t, Opt o, const uint8_t *p, size_t s)
{
    size_t size = s + 5;
    uint8_t tmp[size] = {
        [0] = Cmd::kIAC,
        [1] = Cmd::kSB,
        [2] = o,
    };

    memcpy(tmp + 3, p, s);
    tmp[size-2] = Cmd::kIAC;
    tmp[size-1] = Cmd::kSE;

    sp_tcp_->send(t, tmp, size);
}

}
