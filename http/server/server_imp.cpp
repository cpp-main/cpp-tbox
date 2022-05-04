#include "server_imp.h"

#include <tbox/base/log.h>
#include <tbox/network/buffer.h>

#include "context.h"
#include "middleware.h"

namespace tbox {
namespace http {
namespace server {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

Server::Impl::Impl(Loop *wp_loop) :
    wp_loop_(wp_loop),
    tcp_server_(wp_loop)
{ }

Server::Impl::~Impl()
{
    cleanup();
}

bool Server::Impl::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    if (!tcp_server_.initialize(bind_addr, listen_backlog))
        return false;

    tcp_server_.setConnectedCallback(bind(&Impl::onTcpConnected, this, _1));
    tcp_server_.setDisconnectedCallback(bind(&Impl::onTcpDisconnected, this, _1));
    tcp_server_.setReceiveCallback(bind(&Impl::onTcpReceived, this, _1, _2), 0);

    state_ = State::kInited;
    return true;
}

bool Server::Impl::start()
{
    if (tcp_server_.start()) {
        state_ = State::kRunning;
        return true;
    }
    return false;
}

void Server::Impl::stop()
{
    if (state_ == State::kRunning) {
        tcp_server_.stop();
        state_ = State::kInited;
    }
}

void Server::Impl::cleanup()
{
    if (state_ != State::kNone) {
        stop();

        req_cb_.clear();
        tcp_server_.cleanup();

        for (auto conn : conns_)
            delete conn;
        conns_.clear();

        state_ = State::kNone;
    }
}

void Server::Impl::use(const RequestCallback &cb)
{
    req_cb_.push_back(cb);
}

void Server::Impl::use(Middleware *wp_middleware)
{
    req_cb_.push_back(bind(&Middleware::handle, wp_middleware, _1, _2));
}

void Server::Impl::onTcpConnected(const TcpServer::ConnToken &ct)
{
    auto conn = new Connection;
    tcp_server_.setContext(ct, conn);
    conns_.insert(conn);
}

void Server::Impl::onTcpDisconnected(const TcpServer::ConnToken &ct)
{
    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    conns_.erase(conn);
    delete conn;
}

namespace {
bool IsLastRequest(const Request *req)
{
    auto iter = req->headers.find("Connection");
    if (req->http_ver == HttpVer::k1_0) {
        //! 1.0 版本默认为一连接一请求，需要特定的 Connection: Keep-Alive 才能持处
        if (iter == req->headers.end()) {
            return true;
        } else {
            return iter->second.find("keep-alive") == std::string::npos;
        }
    } else {
        //! 否则为 1.1 及以上的版本，默认为持久连接；除非出现 Connection: close
        if (iter == req->headers.end()) {
            return false;
        } else {
            return iter->second.find("close") != std::string::npos;
        }
    }
}
}

void Server::Impl::onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff)
{
    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));

    //! 如果已被标记为最后的请求，就不应该再有请求来
    if (conn->close_index != numeric_limits<int>::max()) {
        buff.hasReadAll();
        LogNotice("should not recv any data");
        return;
    }

    while (buff.readableSize() > 0) {
        size_t rsize = req_parser_.parse(buff.readableBegin(), buff.readableSize());
        buff.hasRead(rsize);

        if (req_parser_.state() == RequestParser::State::kFinishedAll) {
            Request *req = req_parser_.getRequest();
            LogDbg("[%s]", req->toString().c_str());

            if (IsLastRequest(req)) {
                //! 标记当前请求为close请求
                conn->close_index = conn->req_index;
                //!FIXME:应该关闭读部分
                LogDbg("mark close at %d", conn->close_index);
            }

            auto sp_ctx = make_shared<Context>(this, ct, conn->req_index++, req);
            handle(sp_ctx, 0);

        } else if (req_parser_.state() == RequestParser::State::kFail) {
            tcp_server_.disconnect(ct);
            conns_.erase(conn);
            delete conn;

        } else {
            break;
        }
    }
}

/**
 * 为了保证管道化连接中Respond与Request的顺序一致性，要做特殊处理。
 * 如果所提交的index不是当前需要回复的res_index，那么就先暂存起来，等前面的发送完成后再发送；
 * 如果是，则可以直接回复。然后再将暂存中的未发送的其它数据也一同发送。
 */
void Server::Impl::commitRespond(const TcpServer::ConnToken &ct, int index, string &&content)
{
    if (!tcp_server_.isClientValid(ct))
        return;

    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    if (index == conn->res_index) {
        //! 将当前的数据直接发送出去
        tcp_server_.send(ct, content.data(), content.size());
        LogDbg("[%s]", content.c_str());

        //! 如果当前这个回复是最后一个，则需要断开连接
        if (index == conn->close_index) {
            tcp_server_.disconnect(ct);
            conns_.erase(conn);
            delete conn;
            return;
        }

        ++conn->res_index;

        //! 尝试发送 conn->res_buff 缓冲中的数据
        auto &res_buff = conn->res_buff;
        auto iter = res_buff.find(conn->res_index);

        while (iter != res_buff.end()) {
            tcp_server_.send(ct, iter->second.data(), iter->second.size());
            LogDbg("[%s]", iter->second.c_str());

            //! 如果当前这个回复是最后一个，则需要断开连接
            if (index == conn->close_index) {
                tcp_server_.disconnect(ct);
                conns_.erase(conn);
                delete conn;
                return;
            }

            res_buff.erase(iter);

            ++conn->res_index;
            iter = res_buff.find(conn->res_index);
        }
    } else {
        //! 放入到 conn.res_buff 中暂存
        conn->res_buff[index] = std::move(content);
    }
}

void Server::Impl::handle(ContextSptr sp_ctx, size_t index)
{
    if (index >= req_cb_.size())
        return;

    auto func = req_cb_.at(index);
    if (func)
        func(sp_ctx, std::bind(&Impl::handle, this, sp_ctx, index + 1));
}

}
}
}
