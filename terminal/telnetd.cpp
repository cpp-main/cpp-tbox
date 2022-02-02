#include "telnetd.h"
#include <iostream>
#include <map>
#include <algorithm>
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
        kSGA = 3,
        kSTATUS = 5,
        kCLOCK = 6,
        kTYPE = 24,
        kWINDOW = 31,
        kSPEED = 32,
    };

    bool send(const TcpServer::ClientToken &ct, const void *data_ptr, size_t data_size);

    void sendString(const TcpServer::ClientToken &ct, const std::string &str);
    void sendNego(const TcpServer::ClientToken &ct, Cmd cmd, Opt opt);
    void sendCmd(const TcpServer::ClientToken &ct, Cmd cmd);
    void sendSub(const TcpServer::ClientToken &ct, Opt o, const uint8_t *p, size_t s);

    void onTcpConnected(const TcpServer::ClientToken &ct);
    void onTcpDisconnected(const TcpServer::ClientToken &ct);
    void onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff);

    void onRecvString(const TcpServer::ClientToken &ct, const std::string &str);
    void onRecvNego(const TcpServer::ClientToken &ct, Cmd cmd, Opt opt);
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

    sendNego(ct, kDONT, kECHO);
    sendNego(ct, kDO, kWINDOW);
    sendNego(ct, kDO, kSPEED);
    sendNego(ct, kWILL, kECHO);
    sendNego(ct, kWILL, kSGA);
}

void Telnetd::Impl::onTcpDisconnected(const TcpServer::ClientToken &ct)
{
    cout << ct.id() << " disconnected" << endl;

    auto st = client_to_session_.at(ct);
    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    wp_terminal_->deleteSession(st);
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

void Telnetd::Impl::sendNego(const TcpServer::ClientToken &ct, Cmd cmd, Opt o)
{
    const uint8_t tmp[] = { Cmd::kIAC, cmd, o };
    send(ct, tmp, sizeof(tmp));
}

void Telnetd::Impl::sendCmd(const TcpServer::ClientToken &ct, Cmd cmd)
{
    const uint8_t tmp[] = { Cmd::kIAC, cmd};
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

void Telnetd::Impl::onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff)
{
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << "recv from " << ct.id() << " recv " << buff.readableSize() << ": " << hex_str << endl;

    while (buff.readableSize() != 0) {
        auto begin = buff.readableBegin();
        auto end   = begin + buff.readableSize();
        auto iter  = std::find(begin, end, Cmd::kIAC);
        auto size = iter - begin;
        if (size > 0) {
            onRecvString(ct, std::string(reinterpret_cast<const char *>(begin), size));
        } else {
            if (buff.readableSize() < 2)
                return;

            //! start with IAC
            uint8_t cmd = begin[1];
            if (cmd == Cmd::kWILL || cmd == Cmd::kWONT || cmd == Cmd::kDO || cmd == Cmd::kDONT) {
                if (buff.readableSize() < 3)
                    return;
                onRecvNego(ct, static_cast<Cmd>(cmd), static_cast<Opt>(begin[2]));
                size = 3;

            } else if (cmd == Cmd::kSB) {
                if (buff.readableSize() < 6)
                    return;

                Opt opt = static_cast<Opt>(begin[2]);
                auto cmd_end_iac = std::find(begin + 4, end, Cmd::kIAC);
                if (cmd_end_iac == end)
                    return;
                if ((cmd_end_iac + 1) == end)
                    return;

                onRecvSub(ct, opt, begin + 3, (cmd_end_iac - begin - 3));
                size = cmd_end_iac - begin + 2;

            } else {
                onRecvCmd(ct, static_cast<Cmd>(cmd));
                size = 2;
            }
        }
        buff.hasRead(size);
    }
}

void Telnetd::Impl::onRecvString(const TcpServer::ClientToken &ct, const std::string &str)
{
    auto st = client_to_session_.at(ct);
    wp_terminal_->input(st, str);
}

void Telnetd::Impl::onRecvNego(const TcpServer::ClientToken &ct, Cmd cmd, Opt opt)
{
    LogTrace("cmd:%x, opt:%x", cmd, opt);
}

void Telnetd::Impl::onRecvCmd(const TcpServer::ClientToken &ct, Cmd cmd)
{
    LogTrace("cmd:%x", cmd);
}

void Telnetd::Impl::onRecvSub(const TcpServer::ClientToken &ct, Opt opt, const uint8_t *p, size_t s)
{
    LogTrace("opt:%x, data:%s", opt, util::string::RawDataToHexStr(p, s).c_str());
}

}
