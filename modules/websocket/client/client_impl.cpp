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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "client.h"
#include "client_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include <tbox/network/tcp_connection.h>
#include <tbox/crypto/sha1.h>
#include <tbox/util/base64.h>

#include "../ws_frame_parser.h"
#include "../ws_frame_builder.h"

#include <cstdlib>
#include <cstring>

#undef  MODULE_ID
#define MODULE_ID "tbox.ws.client"

namespace tbox {
namespace websocket {
namespace client {

using namespace std::placeholders;

//! === 静态辅助方法 ===

//! 生成 16 字节随机数并 Base64 编码，作为 Sec-WebSocket-Key
static std::string GenerateSecWebSocketKey()
{
    uint8_t random_bytes[16];
    for (int i = 0; i < 16; ++i)
        random_bytes[i] = static_cast<uint8_t>(rand() & 0xFF);

    return util::base64::Encode(random_bytes, 16);
}

//! 计算 Sec-WebSocket-Accept（与 Server 端一致）
static std::string ComputeWsAcceptKey(const std::string &sec_ws_key)
{
    static const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string combined = sec_ws_key + ws_guid;
    uint8_t digest[20];
    crypto::SHA1::Calc(combined.data(), combined.size(), digest);

    return util::base64::Encode(digest, 20);
}

//! === 生命周期 ===

Client::Impl::Impl(Client *wp_parent, event::Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
{ }

Client::Impl::~Impl()
{
    cleanup();
}

bool Client::Impl::initialize(const network::SockAddr &server_addr, const std::string &url_path)
{
    if (state_ != Client::State::kNone)
        return false;

    server_addr_ = server_addr;
    url_path_ = url_path;

    //! 创建 TcpConnector（保持存活，供重连使用）
    sp_connector_ = new network::TcpConnector(wp_loop_);
    sp_connector_->initialize(server_addr_);
    sp_connector_->setConnectedCallback(std::bind(&Client::Impl::onTcpConnected, this, _1));

    state_ = Client::State::kInited;
    return true;
}

void Client::Impl::setReconnectDelayCalcFunc(const Client::ReconnectDelayCalc &func)
{
    if (sp_connector_ != nullptr)
        sp_connector_->setReconnectDelayCalcFunc(func);
}

bool Client::Impl::start()
{
    if (state_ != Client::State::kInited)
        return false;

    //! 每次连接（含重连）都需要生成新的 Sec-WebSocket-Key
    sec_ws_key_ = GenerateSecWebSocketKey();
    is_closing_ = false;
    frame_parser_.reset();

    //! 开始 TCP 连接（TcpConnector 内部处理重连延迟）
    sp_connector_->start();
    state_ = Client::State::kConnecting;
    return true;
}

void Client::Impl::stop()
{
    if (state_ == Client::State::kNone || state_ == Client::State::kInited)
        return;

    //! 清除 TcpConnection 内部回调，防止断开时回调到 Impl
    if (sp_tcp_conn_ != nullptr) {
        sp_tcp_conn_->setReceiveCallback(nullptr, 0);
        sp_tcp_conn_->setDisconnectedCallback(nullptr);

        sp_tcp_conn_->disconnect();
        auto tcp_conn = sp_tcp_conn_;
        sp_tcp_conn_ = nullptr;
        wp_loop_->runNext([tcp_conn] { CHECK_DELETE_OBJ(tcp_conn); },
            "WsClient::stop, delete tcp_conn");
    }

    //! 停止 TcpConnector（停止正在进行的连接或重连等待）
    if (sp_connector_ != nullptr)
        sp_connector_->stop();

    state_ = Client::State::kInited;
}

void Client::Impl::cleanup()
{
    if (state_ == Client::State::kNone)
        return;

    if (state_ != Client::State::kInited)
        stop();

    CHECK_DELETE_RESET_OBJ(sp_connector_);

    connected_cb_ = nullptr;
    disconnected_cb_ = nullptr;
    message_cb_ = nullptr;
    error_cb_ = nullptr;
    reconnect_enabled_ = true;

    state_ = Client::State::kNone;
}

//! === TCP 连接回调 ===

void Client::Impl::onTcpConnected(network::TcpConnection *tcp_conn)
{
    RECORD_SCOPE();
    LogInfo("tcp connected to %s", tcp_conn->peerAddr().toString().c_str());

    //! 连接成功，TcpConnector 停止但保持存活（不删除，供重连使用）
    sp_connector_->stop();

    //! 保存 TcpConnection，进入握手阶段
    sp_tcp_conn_ = tcp_conn;
    state_ = Client::State::kHandshaking;

    //! 设置 TcpConnection 回调（握手阶段：阈值=0，立即触发）
    sp_tcp_conn_->setReceiveCallback(std::bind(&Client::Impl::onTcpReceived, this, _1), 0);
    sp_tcp_conn_->setDisconnectedCallback(std::bind(&Client::Impl::onTcpDisconnected, this));

    //! 发送握手请求
    sendHandshakeRequest();
}

void Client::Impl::onTcpDisconnected()
{
    RECORD_SCOPE();
    LogInfo("ws client disconnected");

    //! 通知用户
    if (disconnected_cb_) {
        RECORD_SCOPE();
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }

    //! 延后删除 TcpConnection（本函数是 sp_tcp_conn_ 自己调用的）
    auto tobe_delete = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;
    wp_loop_->runNext([tobe_delete] { CHECK_DELETE_OBJ(tobe_delete); },
        "WsClient::onTcpDisconnected, delete tobe_delete");

    state_ = Client::State::kInited;

    //! 自动重连（与 TcpClient 一致：先重连再通知用户）
    if (reconnect_enabled_)
        start();
}

//! === 握手阶段 ===

void Client::Impl::sendHandshakeRequest()
{
    //! RFC 6455 Section 4.1：客户端握手请求
    //! GET /path HTTP/1.1\r\n
    //! Host: host:port\r\n
    //! Upgrade: websocket\r\n
    //! Connection: Upgrade\r\n
    //! Sec-WebSocket-Key: <key>\r\n
    //! Sec-WebSocket-Version: 13\r\n\r\n

    std::string host = server_addr_.toString();

    std::string request =
        "GET " + url_path_ + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Upgrade: websocket\r\n" +
        "Connection: Upgrade\r\n" +
        "Sec-WebSocket-Key: " + sec_ws_key_ + "\r\n" +
        "Sec-WebSocket-Version: 13\r\n\r\n";

    LogDbg("ws client handshake request sent");
    sp_tcp_conn_->send(request.data(), request.size());
}

bool Client::Impl::parseHandshakeResponse(network::Buffer &buff)
{
    //! 查找 \r\n\r\n 分隔符（HTTP 响应头结束标志）
    const char *data = reinterpret_cast<const char*>(buff.readableBegin());
    size_t size = buff.readableSize();

    const char *end = static_cast<const char*>(memmem(data, size, "\r\n\r\n", 4));
    if (end == nullptr)
        return false; //! 响应不完整，等待更多数据

    size_t header_len = end - data + 4;

    //! 简单解析 HTTP 响应行：HTTP/1.1 101 Switching Protocols
    //! 仅检查状态码是否为 101
    std::string header(data, header_len);

    //! 检查状态码 101
    if (header.find("101") == std::string::npos) {
        LogNotice("ws client handshake fail: not 101 response");
        buff.hasRead(header_len);
        return true; //! 解析完成但失败
    }

    //! 检查 Upgrade: websocket
    if (header.find("Upgrade: websocket") == std::string::npos &&
        header.find("Upgrade: WebSocket") == std::string::npos) {
        LogNotice("ws client handshake fail: missing Upgrade: websocket");
        buff.hasRead(header_len);
        return true;
    }

    //! 检查 Connection: Upgrade
    if (header.find("Connection: Upgrade") == std::string::npos) {
        LogNotice("ws client handshake fail: missing Connection: Upgrade");
        buff.hasRead(header_len);
        return true;
    }

    //! 验证 Sec-WebSocket-Accept
    std::string expected_accept = ComputeWsAcceptKey(sec_ws_key_);
    //! 查找 Sec-WebSocket-Accept 头部值
    size_t accept_pos = header.find("Sec-WebSocket-Accept: ");
    if (accept_pos == std::string::npos) {
        LogNotice("ws client handshake fail: missing Sec-WebSocket-Accept");
        buff.hasRead(header_len);
        return true;
    }
    size_t value_start = accept_pos + strlen("Sec-WebSocket-Accept: ");
    size_t value_end = header.find("\r\n", value_start);
    std::string actual_accept = header.substr(value_start, value_end - value_start);

    if (actual_accept != expected_accept) {
        LogNotice("ws client handshake fail: Sec-WebSocket-Accept mismatch");
        buff.hasRead(header_len);
        return true;
    }

    //! 握手成功！消耗响应头，切换到帧通信模式
    buff.hasRead(header_len);
    LogInfo("ws client handshake success");
    onHandshakeSuccess();
    return true;
}

void Client::Impl::onHandshakeSuccess()
{
    state_ = Client::State::kConnected;
    frame_parser_.reset();

    //! 通知用户
    if (connected_cb_) {
        RECORD_SCOPE();
        ++cb_level_;
        connected_cb_();
        --cb_level_;
    }
}

void Client::Impl::onHandshakeFail()
{
    //! 握手失败，断开连接，若启用重连则自动重连
    auto tobe_delete = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;

    //! 延后删除 TcpConnection
    wp_loop_->runNext([tobe_delete] { CHECK_DELETE_OBJ(tobe_delete); },
        "WsClient::onHandshakeFail, delete tobe_delete");

    //! 清除 TcpConnection 回调（防止延后删除期间回调到 Impl）
    //! tobe_delete 已 disconnect，延后删除时不会再回调

    state_ = Client::State::kInited;

    //! 自动重连（与 onTcpDisconnected 一致）
    if (reconnect_enabled_)
        start();
}

//! === 帧通信阶段 ===

void Client::Impl::onWsFrameReceived(network::Buffer &buff)
{
    //! 与 server::WsConnection 的帧解析逻辑相同
    while (buff.readableSize() > 0) {
        size_t consumed = frame_parser_.parse(buff.readableBegin(), buff.readableSize());
        buff.hasRead(consumed);

        if (frame_parser_.state() == WsFrameParser::State::kFinished) {
            WsFrame *frame = frame_parser_.getFrame();
            if (frame != nullptr) {
                switch (frame->opcode) {
                    case WsFrame::OpCode::kText:
                    case WsFrame::OpCode::kBinary:
                    case WsFrame::OpCode::kContinue:
                        if (message_cb_)
                            message_cb_(*frame);
                        break;

                    case WsFrame::OpCode::kClose:
                        //! 收到关闭帧，自动回复关闭帧（掩码）
                        if (!is_closing_) {
                            auto close_frame = WsFrameBuilder::BuildMaskedCloseFrame(frame->closeCode(), frame->closeReason());
                            sp_tcp_conn_->send(close_frame.data(), close_frame.size());
                            is_closing_ = true;
                        }
                        buff.hasReadAll();
                        delete frame;
                        //! 等待 TCP 断开，由 onTcpDisconnected 通知用户并自动重连
                        return;

                    case WsFrame::OpCode::kPing:
                        //! 自动回复 Pong（掩码）
                        pong(frame->payload);
                        break;

                    case WsFrame::OpCode::kPong:
                        //! 收到 Pong，不做特殊处理
                        break;

                    default:
                        LogNotice("ws client unknown opcode: 0x%02x", static_cast<int>(frame->opcode));
                        delete frame;
                        buff.hasReadAll();
                        onError();
                        return;
                }
                delete frame;
            }
        } else if (frame_parser_.state() == WsFrameParser::State::kError) {
            LogNotice("ws client frame parse error");
            buff.hasReadAll();
            onError();
            return;
        } else {
            //! 需要更多数据
            break;
        }
    }
}

//! === TCP 收到数据（握手/帧共用） ===

void Client::Impl::onTcpReceived(network::Buffer &buff)
{
    RECORD_SCOPE();

    if (state_ == Client::State::kHandshaking) {
        //! 握手阶段：解析 HTTP 响应
        bool parsed = parseHandshakeResponse(buff);
        if (parsed) {
            if (state_ == Client::State::kHandshaking) {
                //! parseHandshakeResponse 没有改变 state，说明验证失败
                onHandshakeFail();
            } else {
                //! state 已变为 kConnected，握手成功
                //! buff 中可能还有剩余数据（服务器在 101 后立即发来的帧）
                if (buff.readableSize() > 0)
                    onWsFrameReceived(buff);
            }
        }
        //! parsed == false：响应不完整，等待更多数据
    } else if (state_ == Client::State::kConnected) {
        //! 帧通信阶段
        onWsFrameReceived(buff);
    }
}

void Client::Impl::onError()
{
    //! 出错后断开连接，由 onTcpDisconnected 处理重连
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->disconnect();
}

//! === 通过 ConnToken 操作连接 ===

bool Client::Impl::send(const std::string &text)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != Client::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedTextFrame(text);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool Client::Impl::send(const void *data, size_t len)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != Client::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedBinaryFrame(data, len);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool Client::Impl::sendBinary(const std::vector<uint8_t> &data)
{
    return send(data.data(), data.size());
}

bool Client::Impl::close(uint16_t code, const std::string &reason)
{
    if (sp_tcp_conn_ == nullptr || state_ != Client::State::kConnected)
        return false;

    is_closing_ = true;

    auto frame = WsFrameBuilder::BuildMaskedCloseFrame(code, reason);
    sp_tcp_conn_->send(frame.data(), frame.size());

    //! 延后断开，确保 Close 帧已发送
    wp_loop_->runNext([this] {
        if (sp_tcp_conn_ != nullptr)
            sp_tcp_conn_->disconnect();
    }, "WsClient::close, disconnect");

    return true;
}

bool Client::Impl::ping(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != Client::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedPingFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool Client::Impl::pong(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != Client::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedPongFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool Client::Impl::sendMaskedFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedFrame(opcode, fin, payload, payload_len);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool Client::Impl::isExpired() const
{
    return sp_tcp_conn_ == nullptr || sp_tcp_conn_->isExpired();
}

network::SockAddr Client::Impl::peerAddr() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->peerAddr();
    return server_addr_;
}

void Client::Impl::setContext(void *context, ContextDeleter &&deleter)
{
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->setContext(context, std::move(deleter));
}

void* Client::Impl::getContext() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->getContext();
    return nullptr;
}

}
}
}
