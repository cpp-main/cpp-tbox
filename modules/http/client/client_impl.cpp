/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S E  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "client_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/timer_event.h>
#include <tbox/util/buffer.h>

namespace tbox {
namespace http {
namespace client {

using namespace std;
using namespace std::placeholders;
using namespace event;
using namespace network;

Client::Impl::Impl(Client *wp_parent, Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
  , tcp_client_(wp_loop)
{ }

Client::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
}

bool Client::Impl::initialize(const SockAddr &server_addr)
{
    if (state_ != State::kNone) {
        LogWarn("state not right, can't initialize");
        return false;
    }

    if (!tcp_client_.initialize(server_addr))
        return false;

    tcp_client_.setConnectedCallback(bind(&Impl::onTcpConnected, this));
    tcp_client_.setDisconnectedCallback(bind(&Impl::onTcpDisconnected, this));
    tcp_client_.setReceiveCallback(bind(&Impl::onTcpReceived, this, _1), 0);

    state_ = State::kInited;
    return true;
}

void Client::Impl::setTlsConfig(const TlsConfig &config)
{
    tcp_client_.setTlsConfig(config);
}

bool Client::Impl::start()
{
    if (state_ != State::kInited) {
        LogWarn("state not right, can't start");
        return false;
    }

    if (tcp_client_.start()) {
        state_ = State::kConnecting;
        return true;
    }
    return false;
}

void Client::Impl::stop()
{
    if (state_ == State::kConnected || state_ == State::kConnecting || state_ == State::kReconnWaiting) {
        tcp_client_.stop();
        //! 清理所有缓存和待处理的请求
        failAllPendingRequests(StatusCode::k503_ServiceUnavailable, "client stopped");
        for (auto &item : cached_requests_)
            CHECK_DELETE_RESET_OBJ(item.sp_req);
        cached_requests_.clear();
        res_parser_.reset();
        state_ = State::kInited;
    }
}

void Client::Impl::cleanup()
{
    if (state_ != State::kNone) {
        stop();

        //! 释放所有超时定时器
        for (auto &item : pending_requests_)
            CHECK_DELETE_RESET_OBJ(item.sp_timer);
        pending_requests_.clear();

        //! 释放所有缓存请求
        for (auto &item : cached_requests_)
            CHECK_DELETE_RESET_OBJ(item.sp_req);
        cached_requests_.clear();

        tcp_client_.cleanup();

        state_ = State::kNone;
    }
}

void Client::Impl::request(const Request &req, const RespondCallback &cb)
{
    if (state_ == State::kNone) {
        LogWarn("state is kNone, need initialize first");
        return;
    }

    sendRequest(req, cb);
}

void Client::Impl::request(Method method, const string &path, const RespondCallback &cb)
{
    Request req;
    req.method = method;
    req.http_ver = HttpVer::k1_1;
    req.url.path = path;
    request(req, cb);
}

void Client::Impl::request(Method method, const string &path, const string &body,
                           const Headers &headers, const RespondCallback &cb)
{
    Request req;
    req.method = method;
    req.http_ver = HttpVer::k1_1;
    req.url.path = path;
    req.body = body;
    req.headers = headers;
    request(req, cb);
}

void Client::Impl::setAutoReconnect(bool enable)
{
    auto_reconnect_ = enable;
    tcp_client_.setAutoReconnect(enable);
}

void Client::Impl::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    tcp_client_.setReconnectDelayCalcFunc(func);
}

void Client::Impl::setRequestTimeout(chrono::milliseconds ms)
{
    request_timeout_ms_ = ms;
}

void Client::Impl::setContextLogEnable(bool enable)
{
    context_log_enable_ = enable;
}

void Client::Impl::setConnectedCallback(const ConnectedCallback &cb)
{
    connected_cb_ = cb;
}

void Client::Impl::setConnectFailCallback(const ConnectFailCallback &cb)
{
    connect_fail_cb_ = cb;
}

void Client::Impl::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    disconnected_cb_ = cb;
}

//! ============ TCP 连接回调 ============

void Client::Impl::onTcpConnected()
{
    RECORD_SCOPE();
    state_ = State::kConnected;

    LogDbg("connected");

    //! 发送缓存中的请求
    sendCachedRequests();

    if (connected_cb_) {
        ++cb_level_;
        connected_cb_();
        --cb_level_;
    }
}

void Client::Impl::onTcpConnectFail()
{
    RECORD_SCOPE();
    LogDbg("connect fail");

    //! failAllPendingRequests 中会处理所有 pending 请求
    //! 但 connect fail 时，pending_requests_ 应为空（因为未连接时请求在 cached_requests_ 中）
    //! cached_requests_ 中的请求也要失败回调
    for (auto &item : cached_requests_) {
        Respond res;
        res.http_ver = HttpVer::k1_1;
        res.status_code = StatusCode::k503_ServiceUnavailable;
        res.body = "connect fail";
        if (item.cb) {
            ++cb_level_;
            item.cb(res);
            --cb_level_;
        }
        CHECK_DELETE_RESET_OBJ(item.sp_req);
    }
    cached_requests_.clear();

    if (connect_fail_cb_) {
        ++cb_level_;
        connect_fail_cb_();
        --cb_level_;
    }

    if (!auto_reconnect_) {
        state_ = State::kInited;
    } else {
        state_ = State::kReconnWaiting;
    }
}

void Client::Impl::onTcpDisconnected()
{
    RECORD_SCOPE();
    LogDbg("disconnected");

    //! 对所有 pending 请求执行错误回调
    failAllPendingRequests(StatusCode::k504_GatewayTimeout, "connection lost");

    res_parser_.reset();

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }

    if (auto_reconnect_) {
        state_ = State::kReconnWaiting;
    } else {
        state_ = State::kInited;
    }
}

void Client::Impl::onTcpReceived(Buffer &buff)
{
    RECORD_SCOPE();

    while (buff.readableSize() > 0) {
        size_t rsize = res_parser_.parse(buff.readableBegin(), buff.readableSize());
        buff.hasRead(rsize);

        if (res_parser_.state() == RespondParser::State::kFinishedAll) {
            Respond *sp_respond = res_parser_.getRespond();
            if (sp_respond == nullptr) {
                LogWarn("getRespond() return nullptr, skip");
                continue;
            }

            if (context_log_enable_)
                LogDbg("RES: [%s]", sp_respond->toString().c_str());

            //! 取出 pending_requests_ 队首的请求
            if (!pending_requests_.empty()) {
                auto &pending = pending_requests_.front();

                //! 停止超时定时器
                if (pending.sp_timer != nullptr) {
                    pending.sp_timer->disable();
                    CHECK_DELETE_RESET_OBJ(pending.sp_timer);
                }

                //! 调用回调
                if (pending.cb) {
                    ++cb_level_;
                    pending.cb(*sp_respond);
                    --cb_level_;
                }

                pending_requests_.pop_front();
            } else {
                LogWarn("no pending request for this respond");
            }

            delete sp_respond;

        } else if (res_parser_.state() == RespondParser::State::kFail) {
            LogNotice("parse http respond fail");
            failAllPendingRequests(StatusCode::k502_BadGateway, "respond parse fail");
            //! 断开连接，让 TcpClient 重连
            tcp_client_.stop();
            if (auto_reconnect_) {
                tcp_client_.start();
                state_ = State::kReconnWaiting;
            } else {
                state_ = State::kInited;
            }
            break;

        } else {
            //! 数据不完整，等待更多数据
            break;
        }
    }
}

//! ============ 请求发送 ============

void Client::Impl::sendRequest(const Request &req, const RespondCallback &cb)
{
    if (state_ == State::kConnected) {
        //! 已连接，直接发送
        const string &content = req.toString();
        tcp_client_.send(content.data(), content.size());

        if (context_log_enable_)
            LogDbg("REQ: [%s]", content.c_str());

        //! 创建 PendingRequest，加入队列
        PendingRequest pending;
        pending.req_id = next_req_id_++;
        pending.cb = cb;

        //! 创建超时定时器
        if (request_timeout_ms_.count() > 0) {
            auto sp_timer = wp_loop_->newTimerEvent();
            sp_timer->initialize(request_timeout_ms_, Event::Mode::kOneshot);
            sp_timer->setCallback(bind(&Impl::onRequestTimeout, this, pending.req_id));
            sp_timer->enable();
            pending.sp_timer = sp_timer;
        }

        pending_requests_.push_back(pending);

    } else {
        //! 未连接，缓存请求，等连接建立后发送
        CachedRequest cached;
        cached.sp_req = new Request(req);
        cached.cb = cb;
        cached_requests_.push_back(cached);
    }
}

void Client::Impl::sendCachedRequests()
{
    //! 将缓存中的所有请求发送出去
    for (auto &item : cached_requests_) {
        sendRequest(*item.sp_req, item.cb);
        CHECK_DELETE_RESET_OBJ(item.sp_req);
    }
    cached_requests_.clear();
}

//! ============ 超时与失败处理 ============

void Client::Impl::onRequestTimeout(int req_id)
{
    RECORD_SCOPE();
    LogDbg("request %d timeout", req_id);

    //! 在 pending_requests_ 中查找该请求
    for (auto iter = pending_requests_.begin(); iter != pending_requests_.end(); ++iter) {
        if (iter->req_id == req_id) {
            //! 构造超时响应
            Respond res;
            res.http_ver = HttpVer::k1_1;
            res.status_code = StatusCode::k408_RequestTimeout;
            res.body = "request timeout";

            if (iter->cb) {
                ++cb_level_;
                iter->cb(res);
                --cb_level_;
            }

            //! 清理定时器
            CHECK_DELETE_RESET_OBJ(iter->sp_timer);

            //! 从队列中移除
            //! 注意：超时中间的请求被移除后，后续请求仍可正常接收响应
            //! 但如果服务器按序回复，中间的请求超时意味着后续请求可能也无法收到响应
            //! 这里采用保守策略：仅移除超时的请求，不影响其他请求
            pending_requests_.erase(iter);
            break;
        }
    }
}

void Client::Impl::failAllPendingRequests(StatusCode status_code, const string &message)
{
    for (auto &item : pending_requests_) {
        //! 停止超时定时器
        if (item.sp_timer != nullptr) {
            item.sp_timer->disable();
            CHECK_DELETE_RESET_OBJ(item.sp_timer);
        }

        //! 构造错误响应
        Respond res;
        res.http_ver = HttpVer::k1_1;
        res.status_code = status_code;
        res.body = message;

        if (item.cb) {
            ++cb_level_;
            item.cb(res);
            --cb_level_;
        }
    }
    pending_requests_.clear();

    //! 也处理缓存中的请求
    for (auto &item : cached_requests_) {
        Respond res;
        res.http_ver = HttpVer::k1_1;
        res.status_code = status_code;
        res.body = message;

        if (item.cb) {
            ++cb_level_;
            item.cb(res);
            --cb_level_;
        }
        CHECK_DELETE_RESET_OBJ(item.sp_req);
    }
    cached_requests_.clear();
}

}
}
}
