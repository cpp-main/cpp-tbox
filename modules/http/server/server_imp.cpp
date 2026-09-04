/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "server_imp.h"

#include <algorithm>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/util/buffer.h>
#include <tbox/util/string.h>
#include <tbox/network/tcp_connection.h>

#include "middleware.h"

namespace tbox {
namespace http {
namespace server {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

Server::Impl::Impl(Server *wp_parent, Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
  , tcp_server_(wp_loop)
{ }

Server::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
}

bool Server::Impl::initialize(const network::SockAddr &bind_addr, int listen_backlog)
{
    if (!tcp_server_.initialize(bind_addr, listen_backlog))
        return false;

    tcp_server_.setConnectedCallback(bind(&Impl::onTcpConnected, this, _1));
    tcp_server_.setReceiveCallback(bind(&Impl::onTcpReceived, this, _1, _2), 0);
    tcp_server_.setSendCompleteCallback(bind(&Impl::onTcpSendCompleted, this, _1));

    state_ = State::kInited;
    return true;
}

bool Server::Impl::setTlsConfig(const network::TlsConfig &config)
{
    return tcp_server_.setTlsConfig(config);
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

        mw_cabinet_.foreach([](RequestHandler *ptr) { delete ptr; });
        mw_cabinet_.clear();
        mw_order_.clear();
        tcp_server_.cleanup();

        state_ = State::kNone;
    }
}

MiddlewareToken Server::Impl::use(RequestHandler &&handler)
{
    if (cb_level_ > 0) {
        LogWarn("不能在 next 链中调用 use()，请使用 Loop 的 runNext() 来处理");
        return MiddlewareToken();
    }

    auto token = mw_cabinet_.alloc(new RequestHandler(std::move(handler)));
    mw_order_.push_back(token);
    return token;
}

MiddlewareToken Server::Impl::use(Middleware *wp_middleware)
{
    if (cb_level_ > 0) {
        LogWarn("不能在 next 链中调用 use()，请使用 Loop 的 runNext() 来处理");
        return MiddlewareToken();
    }

    auto token = mw_cabinet_.alloc(new RequestHandler(bind(&Middleware::handle, wp_middleware, _1, _2)));
    mw_order_.push_back(token);
    return token;
}

bool Server::Impl::unuse(const MiddlewareToken &token)
{
    if (cb_level_ > 0) {
        LogWarn("不能在 next 链中调用 unuse()，请使用 Loop 的 runNext() 来处理");
        return false;
    }

    //! 从 Cabinet 中释放
    RequestHandler *handler = mw_cabinet_.free(token);
    if (handler == nullptr)
        return false;     //! token 无效或已释放
    delete handler;

    //! 从 order 中删除该 token（低频 O(n) 操作）
    auto iter = find(mw_order_.begin(), mw_order_.end(), token);
    if (iter != mw_order_.end())
        mw_order_.erase(iter);

    return true;
}

void Server::Impl::onTcpConnected(const TcpServer::ConnToken &ct)
{
    RECORD_SCOPE();
    auto conn = new Connection;
    tcp_server_.setContext(ct, conn,
        [](void* ptr) {
            auto conn = static_cast<Connection*>(ptr);
            CHECK_DELETE_OBJ(conn);
        }
    );
}

namespace {
bool IsLastRequest(const Request *req)
{
    auto iter = http::FindHeader(req->headers, "connection");
    if (req->http_ver == HttpVer::k1_0) {
        //! 1.0 版本默认为一连接一请求，需要特定的 Connection: Keep-Alive 才能持处
        if (iter == req->headers.end()) {
            return true;
        } else {
            return util::string::ToLower(iter->second).find("keep-alive") == std::string::npos;
        }
    } else {
        //! 否则为 1.1 及以上的版本，默认为持久连接；除非出现 Connection: close
        if (iter == req->headers.end()) {
            return false;
        } else {
            return util::string::ToLower(iter->second).find("close") != std::string::npos;
        }
    }
}
}

void Server::Impl::onTcpReceived(const TcpServer::ConnToken &ct, Buffer &buff)
{
    RECORD_SCOPE();
    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    TBOX_ASSERT(conn != nullptr);

    //! 如果已被标记为最后的请求，就不应该再有请求来
    if (conn->close_index != numeric_limits<int>::max()) {
        buff.hasReadAll();
        LogWarn("should not recv any data");
        return;
    }

    //! 如果已被标记为升级请求，停止解析 HTTP 数据
    if (conn->is_upgrade) {
        //! 不消费剩余数据，留给升级后的协议处理
        return;
    }

    while (buff.readableSize() > 0) {
        size_t rsize = conn->req_parser.parse(buff.readableBegin(), buff.readableSize());
        buff.hasRead(rsize);

        if (conn->req_parser.state() == RequestParser::State::kFinishedAll) {
            Request *req = conn->req_parser.getRequest();

            if (context_log_enable_)
                LogDbg("REQ: [%s]", req->toString().c_str());

            auto sp_ctx = make_shared<Context>(wp_parent_, ct, conn->req_index++, req);
            handle(sp_ctx, 0);

            //! 检查是否有协议升级回调（WebSocket、SSE 等）
            //! 如果有，标记连接为升级模式，停止继续解析 HTTP 数据
            if (sp_ctx->res().upgrade_cb) {
                conn->is_upgrade = true;
                //! 升级请求：保留 buffer 中未消费的数据，留给升级后的协议
                break;
            }

            //! 非升级请求：检查是否为最后一个请求
            if (IsLastRequest(req)) {
                conn->close_index = conn->req_index;
                LogDbg("mark close at %d", conn->close_index);

                tcp_server_.shutdown(ct, SHUT_RD);
            }

        } else if (conn->req_parser.state() == RequestParser::State::kFail) {
            LogNotice("parse http from %s fail", tcp_server_.getClientAddress(ct).toString().c_str());
            tcp_server_.disconnect(ct);
            break;

        } else {
            break;
        }
    }
}

void Server::Impl::onTcpSendCompleted(const TcpServer::ConnToken &ct)
{
    RECORD_SCOPE();
    if (!tcp_server_.isClientValid(ct))
        return;

    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    TBOX_ASSERT(conn != nullptr);

    //! 如果最后一个已完成发送，则断开连接
    if (conn->res_index > conn->close_index)
        tcp_server_.disconnect(ct);
}

/**
 * 为了保证管道化连接中Respond与Request的顺序一致性，要做特殊处理。
 * 如果所提交的index不是当前需要回复的res_index，那么就先暂存起来，等前面的发送完成后再发送；
 * 如果是，则可以直接回复。然后再将暂存中的未发送的其它数据也一同发送。
 *
 * 对于协议升级请求（WebSocket 101、SSE 200 等），发送完响应后，
 * 将从HTTP服务器分离TcpConnection，并通过Respond::upgrade_cb回调交给升级协议。
 */
void Server::Impl::commitRespond(const TcpServer::ConnToken &ct, int index, Respond *res)
{
    RECORD_SCOPE();
    if (!tcp_server_.isClientValid(ct)) {
        delete res;
        return;
    }

    //! 处理协议升级请求（WebSocket 101、SSE 200 等）
    //! 触发条件：res->upgrade_cb 已设置（中间件负责设置）
    if (res->upgrade_cb) {
        //! 注意：必须在 std::move(upgrade_cb) 之前调用 toString()
        //! 否则 move 后 upgrade_cb 为空，toString() 会误判为普通响应而添加 Content-Length
        //! SSE 响应添加 Content-Length:0 会导致浏览器认为响应已完成并断开重连
        const string content = res->toString();

        //! 发送响应后，将 TcpConnection 从 HTTP 服务器分离，交给升级协议
        auto upgrade_cb = std::move(res->upgrade_cb);
        delete res;

        tcp_server_.send(ct, content.data(), content.size());
        if (context_log_enable_)
            LogDbg("RES: [%s]", content.c_str());

        //! 当前回调结束后立即执行 detach（使用 runNext，更高效）
        //! 因为 commitRespond() 是在 Loop 线程中执行的
        wp_loop_->runNext([this, ct, upgrade_cb] {
            TcpConnection *tcp_conn = tcp_server_.detachConnection(ct);
            if (tcp_conn != nullptr)
                upgrade_cb(tcp_conn);
            else
                LogWarn("tcp_conn == nullptr");
        }, "HttpUpgrade: detach connection");

        return;
    }

    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    TBOX_ASSERT(conn != nullptr);

    if (index == conn->res_index) {
        //! 将当前的数据直接发送出去
        {
            const string &content = res->toString();
            tcp_server_.send(ct, content.data(), content.size());
            delete res;
            if (context_log_enable_)
                LogDbg("RES: [%s]", content.c_str());
        }

        ++conn->res_index;

        //! 如果当前这个回复是最后一个，则不需要发送缓存中的数据
        if (conn->res_index > conn->close_index)
            return;

        //! 尝试发送 conn->res_buff 缓冲中的数据
        auto &res_buff = conn->res_buff;
        auto iter = res_buff.find(conn->res_index);

        while (iter != res_buff.end()) {
            Respond *res = iter->second;
            {
                const string &content = res->toString();
                tcp_server_.send(ct, content.data(), content.size());
                delete res;
                if (context_log_enable_)
                    LogDbg("RES: [%s]", content.c_str());
            }

            res_buff.erase(iter);
            ++conn->res_index;

            //! 如果当前这个回复是最后一个，则不需要再发送缓存中的数据
            if (conn->res_index > conn->close_index)
                return;

            iter = res_buff.find(conn->res_index);
        }
    } else {
        //! 放入到 conn.res_buff 中暂存
        conn->res_buff[index] = res;
    }
}

void Server::Impl::handle(ContextSptr sp_ctx, size_t index)
{
    RECORD_SCOPE();
    if (index >= mw_order_.size())
        return;

    auto token = mw_order_.at(index);
    RequestHandler *handler_ptr = mw_cabinet_.at(token);

    if (handler_ptr && *handler_ptr) {
        ++cb_level_;
        (*handler_ptr)(sp_ctx, std::bind(&Impl::handle, this, sp_ctx, index + 1));
        --cb_level_;
    } else {
        //! handler 已被移除或为空，跳过，执行下一个
        handle(sp_ctx, index + 1);
    }
}

Server::Impl::Connection::~Connection()
{
    //! 一定要记得清除非发送的Respond
    for (auto & item : res_buff)
        delete item.second;
}

}
}
}
