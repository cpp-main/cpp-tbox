#include "tcp_rpc.h"

#include <iostream>
#include <algorithm>
#include <map>

#include <tbox/util/string.h>
#include <tbox/base/log.h>
#include <tbox/network/tcp_server.h>

#include "../service/telnetd.h"
#include "../connection.h"
#include "../terminal_interact.h"

namespace tbox {
namespace terminal {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;
using namespace util;

class TcpRpc::Impl : public Connection {
  public:
    Impl(Loop *wp_loop, TerminalInteract *wp_terminal);
    virtual ~Impl();

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  public:
    bool send(const SessionToken &st, char ch) override;
    bool send(const SessionToken &st, const std::string &str) override;
    bool endSession(const SessionToken &st) override;
    bool isValid(const SessionToken &st) const override;

  protected:
    bool send(const TcpServer::ClientToken &ct, const void *data_ptr, size_t data_size);

    void onTcpConnected(const TcpServer::ClientToken &ct);
    void onTcpDisconnected(const TcpServer::ClientToken &ct);
    void onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff);
    void onRecvString(const TcpServer::ClientToken &ct, const std::string &str);

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;

    map<SessionToken, TcpServer::ClientToken> session_to_client_;
    map<TcpServer::ClientToken, SessionToken> client_to_session_;
};

TcpRpc::TcpRpc(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    impl_(new Impl(wp_loop, wp_terminal))
{
    assert(impl_ != nullptr);
}

TcpRpc::~TcpRpc()
{
    delete impl_;
}

bool TcpRpc::initialize(const std::string &bind_addr)
{
    return impl_->initialize(bind_addr);
}

bool TcpRpc::start()
{
    return impl_->start();
}

void TcpRpc::stop()
{
    return impl_->stop();
}

void TcpRpc::cleanup()
{
    return impl_->cleanup();
}

////////////////////////////////////////////////////
//
////////////////////////////////////////////////////

TcpRpc::Impl::Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    wp_loop_(wp_loop),
    wp_terminal_(wp_terminal),
    sp_tcp_(new TcpServer(wp_loop))
{
    assert(wp_loop_ != nullptr);
    assert(wp_terminal_ != nullptr);
    assert(sp_tcp_ != nullptr);
}

TcpRpc::Impl::~Impl()
{
    delete sp_tcp_;
}

bool TcpRpc::Impl::initialize(const std::string &bind_addr_str)
{
    auto bind_addr = SockAddr::FromString(bind_addr_str);
    if (!sp_tcp_->initialize(bind_addr))
        return false;

    sp_tcp_->setConnectedCallback(std::bind(&Impl::onTcpConnected, this, _1));
    sp_tcp_->setReceiveCallback(std::bind(&Impl::onTcpReceived, this, _1, _2), 0);
    sp_tcp_->setDisconnectedCallback(std::bind(&Impl::onTcpDisconnected, this, _1));
    return true;
}

bool TcpRpc::Impl::start()
{
    return sp_tcp_->start();
}

void TcpRpc::Impl::stop()
{
    sp_tcp_->stop();
}

void TcpRpc::Impl::cleanup()
{
    sp_tcp_->cleanup();
}

bool TcpRpc::Impl::send(const SessionToken &st, const std::string &str)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, str.c_str(), str.size());
    return true;
}

bool TcpRpc::Impl::send(const SessionToken &st, char ch)
{
    auto ct = session_to_client_.at(st);
    if (st.isNull())
        return false;

    send(ct, &ch, 1);
    return true;
}

bool TcpRpc::Impl::endSession(const SessionToken &st)
{
    auto ct = session_to_client_.at(st);
    if (ct.isNull())
        return false;

    //! 委托执行，否则会出自我销毁的异常
    wp_loop_->run(
        [this, st, ct] {
            client_to_session_.erase(ct);
            session_to_client_.erase(st);
            sp_tcp_->disconnect(ct);
        }
    );

    return true;
}

bool TcpRpc::Impl::isValid(const SessionToken &st) const
{
    return session_to_client_.find(st) != session_to_client_.end();
}

void TcpRpc::Impl::onTcpConnected(const TcpServer::ClientToken &ct)
{
    cout << ct.id() << " connected" << endl;

    auto st = wp_terminal_->newSession(this);
    client_to_session_[ct] = st;
    session_to_client_[st] = ct;

    wp_terminal_->onBegin(st);
}

void TcpRpc::Impl::onTcpDisconnected(const TcpServer::ClientToken &ct)
{
    cout << ct.id() << " disconnected" << endl;

    auto st = client_to_session_.at(ct);
    client_to_session_.erase(ct);
    session_to_client_.erase(st);
    wp_terminal_->deleteSession(st);
}

bool TcpRpc::Impl::send(const TcpServer::ClientToken &ct, const void *data_ptr, size_t data_size)
{
#if 1
    auto hex_str = string::RawDataToHexStr(data_ptr, data_size);
    cout << ct.id() << " << send " << data_size << ": " << hex_str << endl;
#endif
    return sp_tcp_->send(ct, data_ptr, data_size);
}

void TcpRpc::Impl::onTcpReceived(const TcpServer::ClientToken &ct, Buffer &buff)
{
#if 1
    auto hex_str = string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
    cout << ct.id() << " >> recv " << buff.readableSize() << ": " << hex_str << endl;
#endif

    onRecvString(ct, std::string(reinterpret_cast<const char *>(buff.readableBegin()), buff.readableSize()));
    buff.hasReadAll();
}

void TcpRpc::Impl::onRecvString(const TcpServer::ClientToken &ct, const std::string &str)
{
    auto st = client_to_session_.at(ct);
    wp_terminal_->onRecvString(st, str);
}

}
}
