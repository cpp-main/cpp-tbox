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
    bool send(const SessionToken &st, const std::string &str) override;
    bool endSession(const SessionToken &st) override;
    bool isValid(const SessionToken &st) const override;

  protected:
    enum Cmd {
        kEOF = 236,
        kSUSP,
        kABORT,
        kEOR,
        kSE,    //!< Sub end
        kNOP,
        kDM,    //!< Data Mark
        kBRK,   //!< Break
        kIP,    //!< Interrupt Process
        kAO,    //!< Abort output
        kAYT,   //!< Are you here
        kEC,    //!< Erase character
        kEL,    //!< Erase line
        kGA,    //!< Go ahead
        kSB,    //!< Sub begin
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

    bool send(const TcpServer::ClientToken &ct, const void *data_ptr, size_t data_size);

    void sendString(const TcpServer::ClientToken &ct, const std::string &str);

    void sendWill(const TcpServer::ClientToken &ct, Opt opt);
    void sendWont(const TcpServer::ClientToken &ct, Opt opt);
    void sendDo(const TcpServer::ClientToken &ct, Opt opt);
    void sendDont(const TcpServer::ClientToken &ct, Opt opt);

    void sendCmd(const TcpServer::ClientToken &ct, Cmd cmd);
    void sendSub(const TcpServer::ClientToken &ct, Opt o, const uint8_t *p, size_t s);

    void onTcpConnected(const TcpServer::ClientToken &ct);
    void onTcpDisconnected(const TcpServer::ClientToken &ct);
    void onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff);

    void onRecvString(const TcpServer::ClientToken &ct, const std::string &str);

    void onRecvWill(const TcpServer::ClientToken &ct, Opt opt);
    void onRecvWont(const TcpServer::ClientToken &ct, Opt opt);
    void onRecvDo(const TcpServer::ClientToken &ct, Opt opt);
    void onRecvDont(const TcpServer::ClientToken &ct, Opt opt);

    void onRecvCmd(const TcpServer::ClientToken &ct, Cmd cmd);
    void onRecvSub(const TcpServer::ClientToken &ct, Opt opt, const uint8_t *p, size_t s);

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

bool Telnetd::Impl::send(const SessionToken &st, const std::string &str)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    sp_tcp_->send(ct, str.c_str(), str.size());
    return true;
}

bool Telnetd::Impl::endSession(const SessionToken &st)
{
    auto ct = session_to_client_.at(st);
    if (ct.isNull())
        return false;

    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    sp_tcp_->disconnect(ct);
    return true;
}

bool Telnetd::Impl::isValid(const SessionToken &st) const
{
    return session_to_client_.find(st) != session_to_client_.end();
}

void Telnetd::Impl::onTcpConnected(const TcpServer::ClientToken &ct)
{
    cout << ct.id() << " connected" << endl;

    auto st = wp_terminal_->newSession(this);
    client_to_session_[ct] = st;
    session_to_client_[st] = ct;

    sendDont(ct, kECHO);
    sendCmd(ct, kGA);
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::ClientToken &ct)
{
    cout << ct.id() << " disconnected" << endl;

    auto st = client_to_session_.at(ct);
    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    wp_terminal_->deleteSession(st);
}

void Telnetd::Impl::onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff)
{
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << "recv from " << ct.id() << " recv " << buff.readableSize() << ": " << hex_str << endl;

    auto st = client_to_session_.at(ct);
    std::string str(reinterpret_cast<const char*>(buff.readableBegin()), buff.readableSize());
    wp_terminal_->input(st, str);

    buff.hasReadAll();
}

bool Telnetd::Impl::send(const TcpServer::ClientToken &ct, const void *data_ptr, size_t data_size)
{
    auto hex_str = string::RawDataToHexStr(data_ptr, data_size);
    cout << "send to " << ct.id() << " size " << data_size << ": " << hex_str << endl;

    return sp_tcp_->send(ct, data_ptr, data_size);
}

void Telnetd::Impl::sendString(const TcpServer::ClientToken &ct, const std::string &str)
{
    send(ct, str.data(), str.size());
}

void Telnetd::Impl::sendWill(const TcpServer::ClientToken &ct, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, Cmd::kWILL, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendWont(const TcpServer::ClientToken &ct, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, Cmd::kWONT, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendDo(const TcpServer::ClientToken &ct, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, Cmd::kDO, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendDont(const TcpServer::ClientToken &ct, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, Cmd::kDONT, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendCmd(const TcpServer::ClientToken &ct, Cmd c)
{
    const uint8_t tmp[] = { Cmd::kIAC, c };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendSub(const TcpServer::ClientToken &ct, Opt o, const uint8_t *p, size_t s)
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

    send(ct, tmp, size);
}

void Telnetd::Impl::onRecvString(const TcpServer::ClientToken &ct, const std::string &str)
{
    LogUndo();
}

void Telnetd::Impl::onRecvWill(const TcpServer::ClientToken &ct, Opt opt)
{
    LogUndo();
}

void Telnetd::Impl::onRecvWont(const TcpServer::ClientToken &ct, Opt opt)
{
    LogUndo();
}

void Telnetd::Impl::onRecvDo(const TcpServer::ClientToken &ct, Opt opt)
{
    LogUndo();
}

void Telnetd::Impl::onRecvDont(const TcpServer::ClientToken &ct, Opt opt)
{
    LogUndo();
}

void Telnetd::Impl::onRecvCmd(const TcpServer::ClientToken &ct, Cmd cmd)
{
    LogUndo();
}

void Telnetd::Impl::onRecvSub(const TcpServer::ClientToken &ct, Opt opt, const uint8_t *p, size_t s)
{
    LogUndo();
}

}
