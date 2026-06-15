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

#include "middleware.h"

namespace tbox {
namespace http {
namespace server {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

Server::Impl::Impl(Server *wp_parent, Loop *wp_loop) :
    wp_parent_(wp_parent),
    tcp_server_(wp_loop)
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
    RECORD_SCOPE();
    Connection *conn = static_cast<Connection*>(tcp_server_.getContext(ct));
    TBOX_ASSERT(conn != nullptr);

    //! 如果已被标记为最后的请求，就不应该再有请求来
    if (conn->close_index != numeric_limits<int>::max()) {
        buff.hasReadAll();
        LogWarn("should not recv any data");
        return;
    }

    while (buff.readableSize() > 0) {
        size_t rsize = conn->req_parser.parse(buff.readableBegin(), buff.readableSize());
        buff.hasRead(rsize);

        if (conn->req_parser.state() == RequestParser::State::kFinishedAll) {
            Request *req = conn->req_parser.getRequest();

            if (context_log_enable_)
                LogDbg("REQ: [%s]", req->toString().c_str());

            if (IsLastRequest(req)) {
                //! 标记当前请求为close请求
                conn->close_index = conn->req_index;
                LogDbg("mark close at %d", conn->close_index);

                tcp_server_.shutdown(ct, SHUT_RD);
            }

            auto sp_ctx = make_shared<Context>(wp_parent_, ct, conn->req_index++, req);
            handle(sp_ctx, 0);

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
 */
void Server::Impl::commitRespond(const TcpServer::ConnToken &ct, int index, Respond *res)
{
    RECORD_SCOPE();
    if (!tcp_server_.isClientValid(ct)) {
        delete res;
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
