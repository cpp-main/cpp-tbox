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
#include "ws_client.h"
#include "ws_client_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include <tbox/network/tcp_connection.h>
#include <tbox/network/tcp_raw_factory.h>
#include <tbox/crypto/sha1.h>
#include <tbox/util/base64.h>
#include <tbox/util/string.h>

#include "../ws_frame_parser.h"
#include "../ws_frame_builder.h"

#include <cstdlib>
#include <cstring>

#undef  MODULE_ID
#define MODULE_ID "tbox.ws.client"

namespace tbox {
namespace websocket {
namespace client {

//! ODR-used static constexpr 成员须在类外定义（C++11）
constexpr size_t WsClient::Impl::kDefaultFragmentSize;

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

WsClient::Impl::Impl(WsClient *wp_parent, event::Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
{ }

WsClient::Impl::~Impl()
{
    cleanup();
}

bool WsClient::Impl::initialize(const network::SockAddr &server_addr, const std::string &url_path)
{
    if (state_ != WsClient::State::kNone)
        return false;

    server_addr_ = server_addr;
    url_path_ = url_path;

    //! 创建 TcpFactory（默认使用 Raw）和 Connector
    sp_factory_ = new network::TcpRawFactory;
    sp_connector_ = sp_factory_->createConnector(wp_loop_);
    sp_connector_->initialize(server_addr_);
    sp_connector_->setConnectedCallback(std::bind(&WsClient::Impl::onTcpConnected, this, _1));

    state_ = WsClient::State::kInited;
    return true;
}

void WsClient::Impl::setTlsConfig(const network::TlsConfig &config)
{
    if (state_ != WsClient::State::kNone) {
        LogWarn("cannot set TLS config after initialization");
        return;
    }

    if (!config.isValid()) {
        LogWarn("invalid TLS config");
        return;
    }

    //! 替换 factory 和 connector
    CHECK_DELETE_RESET_OBJ(sp_connector_);
    CHECK_DELETE_RESET_OBJ(sp_factory_);

    network::TcpFactory *tls_factory = network::CreateTlsFactory(network::TlsRole::kClient, config);
    if (tls_factory == nullptr) {
        LogWarn("failed to create TLS factory, TLS module may not be linked");
        //! 回退到 Raw Factory
        sp_factory_ = new network::TcpRawFactory;
    } else {
        sp_factory_ = tls_factory;
    }

    sp_connector_ = sp_factory_->createConnector(wp_loop_);
    sp_connector_->initialize(server_addr_);
    sp_connector_->setConnectedCallback(std::bind(&WsClient::Impl::onTcpConnected, this, _1));
}

void WsClient::Impl::setReconnectDelayCalcFunc(const WsClient::ReconnectDelayCalc &func)
{
    if (sp_connector_ != nullptr)
        sp_connector_->setReconnectDelayCalcFunc(func);
}

bool WsClient::Impl::start()
{
    if (state_ != WsClient::State::kInited)
        return false;

    //! 每次连接（含重连）都需要生成新的 Sec-WebSocket-Key
    sec_ws_key_ = GenerateSecWebSocketKey();
    is_closing_ = false;
    frame_parser_.reset();

    //! 清理分片缓存
    fragment_buffer_.clear();
    is_fragmenting_ = false;

    //! 开始 TCP 连接（TcpConnector 内部处理重连延迟）
    sp_connector_->start();
    state_ = WsClient::State::kConnecting;
    return true;
}

void WsClient::Impl::stop()
{
    if (state_ == WsClient::State::kNone || state_ == WsClient::State::kInited)
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

    state_ = WsClient::State::kInited;
}

void WsClient::Impl::cleanup()
{
    if (state_ == WsClient::State::kNone)
        return;

    if (state_ != WsClient::State::kInited)
        stop();

    CHECK_DELETE_RESET_OBJ(sp_connector_);
    CHECK_DELETE_RESET_OBJ(sp_factory_);

    CHECK_DELETE_RESET_OBJ(sp_ping_timer_);
    CHECK_DELETE_RESET_OBJ(sp_pong_timer_);

    connected_cb_ = nullptr;
    disconnected_cb_ = nullptr;
    text_message_cb_ = nullptr;
    binary_message_cb_ = nullptr;
    error_cb_ = nullptr;
    reconnect_enabled_ = true;

    //! 清理分片缓存
    fragment_buffer_.clear();
    is_fragmenting_ = false;

    state_ = WsClient::State::kNone;
}

//! === TCP 连接回调 ===

void WsClient::Impl::onTcpConnected(network::TcpConnection *tcp_conn)
{
    RECORD_SCOPE();
    LogInfo("tcp connected to %s", tcp_conn->peerAddr().toString().c_str());

    //! 连接成功，TcpConnector 停止但保持存活（不删除，供重连使用）
    sp_connector_->stop();

    //! 保存 TcpConnection，进入握手阶段
    sp_tcp_conn_ = tcp_conn;
    state_ = WsClient::State::kHandshaking;

    //! 设置 TcpConnection 回调（握手阶段：阈值=0，立即触发）
    sp_tcp_conn_->setReceiveCallback(std::bind(&WsClient::Impl::onTcpReceived, this, _1), 0);
    sp_tcp_conn_->setDisconnectedCallback(std::bind(&WsClient::Impl::onTcpDisconnected, this));

    //! 发送握手请求
    sendHandshakeRequest();
}

void WsClient::Impl::onTcpDisconnected()
{
    RECORD_SCOPE();
    LogInfo("ws client disconnected");

    //! 清理分片缓存
    fragment_buffer_.clear();
    is_fragmenting_ = false;

    //! 禁用心跳定时器
    if (sp_ping_timer_ != nullptr)
        sp_ping_timer_->disable();
    if (sp_pong_timer_ != nullptr)
        sp_pong_timer_->disable();
    is_pong_pending_ = false;

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

    state_ = WsClient::State::kInited;

    //! 自动重连（与 TcpClient 一致：先重连再通知用户）
    if (reconnect_enabled_)
        start();
}

//! === 握手阶段 ===

void WsClient::Impl::sendHandshakeRequest()
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
        "Sec-WebSocket-Version: 13\r\n";

    //! RFC 7692：若 prefer_compression_=true，请求压缩扩展
    //! 必须声明 client_no_context_takeover 和 server_no_context_takeover
    //! 与我们的实现一致（每条消息独立压缩）
    if (prefer_compression_) {
        request += "Sec-WebSocket-Extensions: permessage-deflate; client_no_context_takeover; server_no_context_takeover\r\n";
    }

    request += "\r\n";

    LogDbg("ws client handshake request sent");
    sp_tcp_conn_->send(request.data(), request.size());
}

bool WsClient::Impl::parseHandshakeResponse(network::Buffer &buff)
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

    //! RFC 7692：检查压缩协商结果
    //! 若客户端请求了压缩且服务器同意了 permessage-deflate
    if (prefer_compression_ &&
        header.find("Sec-WebSocket-Extensions: permessage-deflate") != std::string::npos) {
        //! 服务器同意压缩
        compression_config_.enabled = true;
        compression_config_.no_context_takeover = true;
        compression_config_.max_window_bits = 15;
        LogInfo("ws client compression agreed: permessage-deflate");
    } else {
        //! 服务器不同意压缩，或客户端未请求
        compression_config_.enabled = false;
    }

    onHandshakeSuccess();
    return true;
}

void WsClient::Impl::onHandshakeSuccess()
{
    state_ = WsClient::State::kConnected;
    frame_parser_.reset();

    //! 初始化压缩器
    if (compression_config_.enabled) {
        if (!compressor_.initialize(compression_config_)) {
            LogErr("WsClient compressor init fail, fallback to no compression");
            compression_config_.enabled = false;
        }
    }

    //! 初始化 Ping/Pong 心跳定时器
    //! 每次连接（含重连）都重新创建定时器
    CHECK_DELETE_RESET_OBJ(sp_ping_timer_);
    CHECK_DELETE_RESET_OBJ(sp_pong_timer_);
    is_pong_pending_ = false;

    if (ping_interval_ > 0) {
        sp_ping_timer_ = wp_loop_->newTimerEvent();
        sp_ping_timer_->initialize(std::chrono::seconds(ping_interval_), event::Event::Mode::kPersist);
        sp_ping_timer_->setCallback(std::bind(&WsClient::Impl::onPingTimerFired, this));
        sp_ping_timer_->enable();

        if (ping_timeout_ > 0) {
            sp_pong_timer_ = wp_loop_->newTimerEvent();
            sp_pong_timer_->initialize(std::chrono::seconds(ping_timeout_), event::Event::Mode::kOneshot);
            sp_pong_timer_->setCallback(std::bind(&WsClient::Impl::onPongTimeoutFired, this));
        }
    }

    //! 通知用户
    if (connected_cb_) {
        RECORD_SCOPE();
        ++cb_level_;
        connected_cb_();
        --cb_level_;
    }
}

void WsClient::Impl::onHandshakeFail()
{
    //! 握手失败，断开连接，若启用重连则自动重连
    auto tobe_delete = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;

    //! 延后删除 TcpConnection
    wp_loop_->runNext([tobe_delete] { CHECK_DELETE_OBJ(tobe_delete); },
        "WsClient::onHandshakeFail, delete tobe_delete");

    //! 清除 TcpConnection 回调（防止延后删除期间回调到 Impl）
    //! tobe_delete 已 disconnect，延后删除时不会再回调

    state_ = WsClient::State::kInited;

    //! 自动重连（与 onTcpDisconnected 一致）
    if (reconnect_enabled_)
        start();
}

//! === 帧通信阶段 ===

//! 将完整数据交付给业务层
//! data 为解压后的完整数据（若不需要解压则为原始 payload）
//! opcode 为消息类型（kText 或 kBinary）
void WsClient::Impl::deliverMessage(WsFrame::OpCode opcode, std::string &data)
{
    if (opcode == WsFrame::OpCode::kText) {
        if (text_message_cb_) {
            ++cb_level_;
            text_message_cb_(std::move(data));
            --cb_level_;
        }
    } else if (opcode == WsFrame::OpCode::kBinary) {
        //! 将 std::string 转换为 std::vector<uint8_t>
        std::vector<uint8_t> vec(data.begin(), data.end());
        if (binary_message_cb_) {
            ++cb_level_;
            binary_message_cb_(std::move(vec));
            --cb_level_;
        }
    }
}

void WsClient::Impl::onWsFrameReceived(network::Buffer &buff)
{
    //! 与 server::WsConnection 的帧解析逻辑相同：分片数据先缓存，接收完整后统一解压再回调
    while (buff.readableSize() > 0) {
        size_t consumed = frame_parser_.parse(buff.readableBegin(), buff.readableSize());
#if 1
        auto hex_str = util::string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
        LogTrace("hex: %s, consumed:%u", hex_str.c_str(), consumed);
#endif
        buff.hasRead(consumed);

        if (frame_parser_.state() == WsFrameParser::State::kFinished) {
            WsFrame *frame = frame_parser_.getFrame();
            if (frame != nullptr) {
                //! ===== 控制帧处理（Close/Ping/Pong 不受分片状态影响） =====
                if (frame->isControlFrame()) {
                    switch (frame->opcode) {
                        case WsFrame::OpCode::kClose:
                            //! 收到关闭帧，自动回复关闭帧（掩码）
                            if (!is_closing_) {
                                auto close_frame = WsFrameBuilder::BuildMaskedCloseFrame(frame->closeCode(), frame->closeReason());
                                sp_tcp_conn_->send(close_frame.data(), close_frame.size());
                                is_closing_ = true;
                            }
                            buff.hasReadAll();
                            //! 清理分片缓存
                            fragment_buffer_.clear();
                            is_fragmenting_ = false;
                            delete frame;
                            //! 等待 TCP 断开，由 onTcpDisconnected 通知用户并自动重连
                            return;

                        case WsFrame::OpCode::kPing:
                            //! 自动回复 Pong（掩码）
                            pong(frame->payload);
                            break;

                        case WsFrame::OpCode::kPong:
                            //! 心跳：收到 Pong，取消超时定时器
                            if (is_pong_pending_) {
                                is_pong_pending_ = false;
                                if (sp_pong_timer_ != nullptr)
                                    sp_pong_timer_->disable();
                            }
                            break;

                        default:
                            LogNotice("ws client unknown control opcode: 0x%02x", static_cast<int>(frame->opcode));
                            delete frame;
                            buff.hasReadAll();
                            fragment_buffer_.clear();
                            is_fragmenting_ = false;
                            onError();
                            return;
                    }
                    delete frame;
                    continue;   //! 控制帧处理完毕，继续解析下一个帧
                }

                //! ===== 数据帧处理（TEXT / BINARY / CONTINUE） =====
                //! 核心逻辑：分片数据先缓存，接收完整后统一解压再回调
                //! 原因：压缩数据不能逐片解压，必须拼接完整后才能解压

                if (frame->opcode == WsFrame::OpCode::kText ||
                    frame->opcode == WsFrame::OpCode::kBinary) {
                    //! 新消息的首帧
                    if (is_fragmenting_) {
                        //! 正在接收分片消息时又收到新消息首帧，协议违规
                        LogNotice("ws client protocol error: new data frame while fragmenting");
                        delete frame;
                        buff.hasReadAll();
                        fragment_buffer_.clear();
                        is_fragmenting_ = false;
                        onError();
                        return;
                    }

                    if (frame->fin) {
                        //! 单帧完整消息（无分片）
                        bool is_need_decompress = frame->rsv1 && compression_config_.enabled;
                        if (is_need_decompress) {
                            std::string decompressed = compressor_.decompress(frame->payload);
                            if (!decompressed.empty()) {
                                frame->payload = std::move(decompressed);
                            } else {
                                //! 解压失败
                                LogNotice("ws client decompress fail");
                                delete frame;
                                buff.hasReadAll();
                                onError();
                                return;
                            }
                        }

                        //! 交付完整消息给业务层
                        deliverMessage(frame->opcode, frame->payload);
                        delete frame;

                    } else {
                        //! 分片消息的首帧（fin=false）
                        //! 记录原始 opcode 和是否需要解压，缓存 payload
                        is_fragmenting_ = true;
                        fragment_opcode_ = frame->opcode;
                        fragment_need_decompress_ = frame->rsv1 && compression_config_.enabled;
                        fragment_buffer_ = std::move(frame->payload);
                        delete frame;
                    }

                } else if (frame->opcode == WsFrame::OpCode::kContinue) {
                    //! 分片消息的后续帧
                    if (!is_fragmenting_) {
                        //! 没有首帧却收到续帧，协议违规
                        LogNotice("ws client protocol error: continue frame without fragment start");
                        delete frame;
                        buff.hasReadAll();
                        onError();
                        return;
                    }

                    //! 将本片 payload 追加到缓存区
                    fragment_buffer_.append(frame->payload);

                    if (frame->fin) {
                        //! 最后一帧（fin=true），消息完整
                        //! 对完整数据统一解压，然后回调业务层
                        if (fragment_need_decompress_) {
                            std::string decompressed = compressor_.decompress(fragment_buffer_);
                            if (!decompressed.empty()) {
                                fragment_buffer_ = std::move(decompressed);
                            } else {
                                //! 解压失败
                                LogNotice("ws client decompress fail");
                                delete frame;
                                buff.hasReadAll();
                                fragment_buffer_.clear();
                                is_fragmenting_ = false;
                                onError();
                                return;
                            }
                        }

                        //! 交付完整消息给业务层
                        deliverMessage(fragment_opcode_, fragment_buffer_);

                        //! 重置分片状态
                        fragment_buffer_.clear();
                        is_fragmenting_ = false;
                    }
                    //! fin=false: 继续缓存，不回调

                    delete frame;

                } else {
                    //! 未知数据帧 opcode
                    LogNotice("ws client unknown opcode: 0x%02x", static_cast<int>(frame->opcode));
                    delete frame;
                    buff.hasReadAll();
                    fragment_buffer_.clear();
                    is_fragmenting_ = false;
                    onError();
                    return;
                }
            }
        } else if (frame_parser_.state() == WsFrameParser::State::kError) {
            LogNotice("ws client frame parse error");
            buff.hasReadAll();
            fragment_buffer_.clear();
            is_fragmenting_ = false;
            onError();
            return;
        } else {
            //! 需要更多数据
            break;
        }
    }
}

//! === TCP 收到数据（握手/帧共用） ===

void WsClient::Impl::onTcpReceived(network::Buffer &buff)
{
    RECORD_SCOPE();

    if (state_ == WsClient::State::kHandshaking) {
        //! 握手阶段：解析 HTTP 响应
        bool parsed = parseHandshakeResponse(buff);
        if (parsed) {
            if (state_ == WsClient::State::kHandshaking) {
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
    } else if (state_ == WsClient::State::kConnected) {
        //! 帧通信阶段
        onWsFrameReceived(buff);
    }
}

void WsClient::Impl::onError()
{
    //! 清理分片缓存
    fragment_buffer_.clear();
    is_fragmenting_ = false;

    //! 出错后断开连接，由 onTcpDisconnected 处理重连
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->disconnect();
}

//! === 通过 ConnToken 操作连接 ===

bool WsClient::Impl::send(const std::string &text)
{
    return sendData(WsFrame::OpCode::kText, text.data(), text.size());
}

bool WsClient::Impl::send(const char *str)
{
    return sendData(WsFrame::OpCode::kText, str, strlen(str));
}

bool WsClient::Impl::send(const void *data, size_t len)
{
    return sendData(WsFrame::OpCode::kBinary, data, len);
}

bool WsClient::Impl::send(const std::vector<uint8_t> &data)
{
    return sendData(WsFrame::OpCode::kBinary, data.data(), data.size());
}

bool WsClient::Impl::close(uint16_t code, const std::string &reason)
{
    if (sp_tcp_conn_ == nullptr || state_ != WsClient::State::kConnected)
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

bool WsClient::Impl::ping(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != WsClient::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedPingFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool WsClient::Impl::pong(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != WsClient::State::kConnected)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedPongFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool WsClient::Impl::sendMaskedFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildMaskedFrame(opcode, fin, payload, payload_len);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

//! 统一发送数据：前置检查 → 压缩(如需要) → sendFragmented
//! opcode 为 kText 或 kBinary
bool WsClient::Impl::sendData(WsFrame::OpCode opcode, const void *data_ptr, size_t data_len)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != WsClient::State::kConnected)
        return false;

    //! 压缩协商达成时，压缩数据
    if (compression_config_.enabled && compressor_.isInitialized()) {
        std::string compressed = compressor_.compress(data_ptr, data_len);
        if (!compressed.empty()) {
            return sendFragmented(opcode, compressed.data(), compressed.size(), true);
        }
        //! 压缩失败，回退到不压缩
        LogNotice("ws client compress fail, fallback to uncompressed");
    }

    return sendFragmented(opcode, data_ptr, data_len, false);
}

//! 分片发送 payload（客户端版本，掩码）
//! 若 payload 大小超过 fragment_size_，则分片发送：
//! - 首帧：原始 opcode，fin=false，rsv1=is_compressed（掩码）
//! - 中间帧：kContinue，fin=false（掩码）
//! - 末帧：kContinue，fin=true（掩码）
//! 若 payload 大小不超过 fragment_size_，则单帧发送
bool WsClient::Impl::sendFragmented(WsFrame::OpCode opcode, const void *payload, size_t payload_len, bool is_compressed)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    //! 单帧即可发送（fragment_size_ 为 0 时表示不分片）
    if (fragment_size_ == 0 || payload_len <= fragment_size_) {
        auto frame = WsFrameBuilder::BuildMaskedFrame(opcode, true, payload, payload_len, nullptr, is_compressed);
        return sp_tcp_conn_->send(frame.data(), frame.size());
    }

    //! 分片发送
    const uint8_t *data = static_cast<const uint8_t*>(payload);
    size_t offset = 0;

    //! 首帧：原始 opcode，fin=false，rsv1=is_compressed（掩码）
    size_t first_chunk = fragment_size_;
    auto frame = WsFrameBuilder::BuildMaskedFrame(opcode, false, data, first_chunk, nullptr, is_compressed);
    if (!sp_tcp_conn_->send(frame.data(), frame.size()))
        return false;

    offset += first_chunk;

    //! 中间帧与末帧：opcode=kContinue，rsv1=false（掩码）
    while (offset < payload_len) {
        size_t remaining = payload_len - offset;
        size_t chunk_size = std::min(remaining, fragment_size_);
        bool is_last = (offset + chunk_size == payload_len);

        auto cont_frame = WsFrameBuilder::BuildMaskedFrame(WsFrame::OpCode::kContinue, is_last,
                                                           data + offset, chunk_size, nullptr, false);
        if (!sp_tcp_conn_->send(cont_frame.data(), cont_frame.size()))
            return false;

        offset += chunk_size;
    }

    return true;
}

bool WsClient::Impl::isExpired() const
{
    return sp_tcp_conn_ == nullptr || sp_tcp_conn_->isExpired();
}

network::SockAddr WsClient::Impl::peerAddr() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->peerAddr();
    return server_addr_;
}

void WsClient::Impl::setContext(void *context, ContextDeleter &&deleter)
{
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->setContext(context, std::move(deleter));
}

void* WsClient::Impl::getContext() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->getContext();
    return nullptr;
}

//! Ping 定时器触发：发送 Ping，启动 Pong 超时检测
void WsClient::Impl::onPingTimerFired()
{
    if (is_closing_ || sp_tcp_conn_ == nullptr || state_ != WsClient::State::kConnected)
        return;

    //! 发送 Ping 帧
    ping("");

    //! 如果有超时检测，标记等待 Pong 并启动超时定时器
    if (ping_timeout_ > 0 && sp_pong_timer_ != nullptr) {
        is_pong_pending_ = true;
        sp_pong_timer_->enable();
    }
}

//! Pong 超时触发：未收到 Pong 回复，判定连接已断开
void WsClient::Impl::onPongTimeoutFired()
{
    if (is_closing_)
        return;

    LogNotice("ws client pong timeout, closing connection");
    is_pong_pending_ = false;
    close(1006, "pong timeout");
}

}
}
}
