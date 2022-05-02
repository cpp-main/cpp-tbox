#include "server.h"
#include <tbox/base/log.h>
#include <tbox/network/tcp_server.h>
#include <tbox/network/buffer.h>
#include <map>

namespace tbox {
namespace http {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

class Server::Impl {
  public:
    Impl(event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

  public:
    void use(const RequestCallback &cb);
    void use(Middleware *wp_middleware);

    void commitRespond(const ConnToken &ct, int index, string &content);

  private:

    void onTcpConnected(const ConnToken &ct);
    void onTcpDisconnected(const ConnToken &ct);
    void onTcpReceived(const ConnToken &ct, Buffer &buff);

    struct Connection {
        int req_index = 0;
        int res_index = 0;
        map<int, string> res_buff;
    };

    void handle(RequestSptr req, RespondSptr res, size_t index);

  private:
    Loop *wp_loop_;
    TcpServer tcp_server_;
    vector<RequestCallback> req_cb_;
    map<ConnToken, Connection> conn_map_;
};

Server::Impl::Impl(Loop *wp_loop) :
    wp_loop_(wp_loop),
    tcp_server_(wp_loop)
{ }

Server::Impl::~Impl()
{ }

bool Server::Impl::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    if (!tcp_server_.initialize(bind_addr, listen_backlog))
        return false;

    tcp_server_.setConnectedCallback(bind(&Impl::onTcpConnected, this, _1));
    tcp_server_.setDisconnectedCallback(bind(&Impl::onTcpDisconnected, this, _1));
    tcp_server_.setReceiveCallback(bind(&Impl::onTcpReceived, this, _1, _2), 0);

    return true;
}

bool Server::Impl::start()
{
    return tcp_server_.start();
}

void Server::Impl::stop()
{
    return tcp_server_.stop();
}

void Server::Impl::cleanup()
{
    req_cb_.clear();
    tcp_server_.cleanup();
}

void Server::Impl::use(const RequestCallback &cb)
{
    req_cb_.push_back(cb);
}

void Server::Impl::use(Middleware *wp_middleware)
{
    req_cb_.push_back(bind(&Middleware::handle, wp_middleware, _1, _2, _3));
}

void Server::Impl::onTcpConnected(const ConnToken &ct)
{
    LogUndo();
}

void Server::Impl::onTcpDisconnected(const ConnToken &ct)
{
    LogUndo();
}

void Server::Impl::onTcpReceived(const ConnToken &ct, Buffer &buff)
{
    LogUndo();
}

/**
 * 为了保证管道化连接中Respond与Request的顺序一致性，要做特殊处理。
 * 如果所提交的index不是当前需要回复的res_index，那么就先暂存起来，等前面的发送完成后再发送；
 * 如果是，则可以直接回复。然后再将暂存中的未发送的其它数据也一同发送。
 */
void Server::Impl::commitRespond(const ConnToken &ct, int index, string &content)
{
    if (!tcp_server_.isClientValid(ct))
        return;

    auto &conn = conn_map_.at(ct);
    if (index == conn.res_index) {
        tcp_server_.send(ct, content.data(), content.size());
        ++conn.res_index;

        //! 尝试发送 conn.res_buff 缓冲中的数据
        auto &res_buff = conn.res_buff;
        auto iter = res_buff.find(conn.res_index);

        while (iter != res_buff.end()) {
            tcp_server_.send(ct, iter->second.data(), iter->second.size());
            res_buff.erase(iter);

            ++conn.res_index;
            iter = res_buff.find(conn.res_index);
        }
    } else {
        //! 放入到 conn.res_buff 中暂存
        conn.res_buff[index] = std::move(content);
    }
}

void Server::Impl::handle(RequestSptr req, RespondSptr res, size_t index)
{
    if (index >= req_cb_.size())
        return;

    auto func = req_cb_.at(index);
    if (func)
        func(req, res, std::bind(&Impl::handle, this, req, res, index + 1));
}

////////////////////////////////
// 包装
////////////////////////////////

Server::Server(Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{ }

Server::~Server()
{
    delete impl_;
}

bool Server::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    return impl_->initialize(bind_addr, listen_backlog);
}

bool Server::start()
{
    return impl_->start();
}

void Server::stop()
{
    impl_->stop();
}

void Server::cleanup()
{
    impl_->cleanup();
}

void Server::use(const RequestCallback &cb)
{
    impl_->use(cb);
}

void Server::use(Middleware *wp_middleware)
{
    impl_->use(wp_middleware);
}

}
}
