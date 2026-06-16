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
#include "ws_connection.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>

#include "../ws_frame_parser.h"
#include "../ws_frame_builder.h"

namespace tbox {
namespace websocket {
namespace server {

using namespace std::placeholders;

WsConnection::WsConnection(event::Loop *wp_loop, network::TcpConnection *tcp_conn, const std::string &url)
  : wp_loop_(wp_loop)
  , sp_tcp_conn_(tcp_conn)
  , url_(url)
{
    TBOX_ASSERT(wp_loop != nullptr);
    TBOX_ASSERT(tcp_conn != nullptr);

    //! 设置 TcpConnection 的回调
    sp_tcp_conn_->setReceiveCallback(std::bind(&WsConnection::onTcpReceived, this, _1), 0);
    sp_tcp_conn_->setDisconnectedCallback(std::bind(&WsConnection::onTcpDisconnected, this));
    sp_tcp_conn_->setSendCompleteCallback(std::bind(&WsConnection::onTcpSendCompleted, this));
}

WsConnection::~WsConnection()
{
    TBOX_ASSERT(cb_level_ == 0);

    if (sp_tcp_conn_ == nullptr)
      return;

    //! 先取消 TcpConnection 的回调，防止断开时回调到已销毁的 WsConnection
    sp_tcp_conn_->setReceiveCallback(nullptr, 0);
    sp_tcp_conn_->setDisconnectedCallback(nullptr);
    sp_tcp_conn_->setSendCompleteCallback(nullptr);

    sp_tcp_conn_->disconnect();
    auto tcp_conn = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;
    wp_loop_->runNext([tcp_conn] { CHECK_DELETE_OBJ(tcp_conn); },
        "WsConnection::~WsConnection, delete tcp_conn");
}

bool WsConnection::send(const std::string &text)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildTextFrame(text);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool WsConnection::send(const void *data, size_t len)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildBinaryFrame(data, len);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool WsConnection::sendBinary(const std::vector<uint8_t> &data)
{
    return send(data.data(), data.size());
}

bool WsConnection::close(uint16_t code, const std::string &reason)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    is_closing_ = true;

    auto frame = WsFrameBuilder::BuildCloseFrame(code, reason);
    sp_tcp_conn_->send(frame.data(), frame.size());

    //! 延后断开，确保 Close 帧已发送
    wp_loop_->runNext([this] {
        if (sp_tcp_conn_ != nullptr)
            sp_tcp_conn_->disconnect();
    }, "WsConnection::close, disconnect");

    return true;
}

bool WsConnection::ping(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildPingFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

bool WsConnection::pong(const std::string &data)
{
    if (is_closing_ || sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildPongFrame(data);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

network::SockAddr WsConnection::peerAddr() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->peerAddr();
    return network::SockAddr();
}

std::string WsConnection::getUrl() const
{
    return url_;
}

bool WsConnection::isExpired() const
{
    return sp_tcp_conn_ == nullptr || sp_tcp_conn_->isExpired();
}

void WsConnection::setContext(void *context, ContextDeleter &&deleter)
{
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->setContext(context, std::move(deleter));
}

void* WsConnection::getContext() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->getContext();
    return nullptr;
}

bool WsConnection::sendFrame(WsFrame::OpCode opcode, bool fin,
                             const void *payload, size_t payload_len)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    auto frame = WsFrameBuilder::BuildFrame(opcode, fin, payload, payload_len);
    return sp_tcp_conn_->send(frame.data(), frame.size());
}

void WsConnection::onTcpReceived(network::Buffer &buff)
{
    //! 从缓冲区中逐步解析 WebSocket 帧
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
                        if (message_cb_) {
                            ++cb_level_;
                            message_cb_(*frame);
                            --cb_level_;
                        }
                        break;

                    case WsFrame::OpCode::kClose:
                        //! 收到关闭帧，自动回复关闭帧
                        if (!is_closing_) {
                            auto close_frame = WsFrameBuilder::BuildCloseFrame(frame->closeCode(), frame->closeReason());
                            sp_tcp_conn_->send(close_frame.data(), close_frame.size());
                            is_closing_ = true;
                        }
                        //! 不再处理后续数据
                        buff.hasReadAll();
                        delete frame;
                        if (close_cb_) {
                            ++cb_level_;
                            close_cb_();
                            --cb_level_;
                        }
                        return;

                    case WsFrame::OpCode::kPing:
                        //! 自动回复 Pong
                        pong(frame->payload);
                        if (ping_cb_) {
                            ++cb_level_;
                            ping_cb_(frame->payload);
                            --cb_level_;
                        }
                        break;

                    case WsFrame::OpCode::kPong:
                        if (pong_cb_) {
                            ++cb_level_;
                            pong_cb_(frame->payload);
                            --cb_level_;
                        }
                        break;

                    default:
                        LogNotice("unknown ws opcode: 0x%02x", static_cast<int>(frame->opcode));
                        delete frame;
                        buff.hasReadAll();
                        if (error_cb_) {
                            ++cb_level_;
                            error_cb_();
                            --cb_level_;
                        }

                        return;
                }
                delete frame;
            }
        } else if (frame_parser_.state() == WsFrameParser::State::kError) {
            LogNotice("ws frame parse error");
            buff.hasReadAll();
            if (error_cb_) {
                ++cb_level_;
                error_cb_();
                --cb_level_;
            }
            return;
        } else {
            //! 需要更多数据
            break;
        }
    }
}

void WsConnection::onTcpDisconnected()
{
    LogInfo("ws disconnected");

    if (close_cb_) {
        ++cb_level_;
        close_cb_();
        --cb_level_;
    }

    //! 清理 TcpConnection：先断开再延后删除
    //! 断空指针，防止析构函数重复操作已删除的对象
    //! 必须要 close_cb_() 之后才能清理，否则回调中 getContext() 拿到是空的
    auto tcp_conn = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;
    wp_loop_->runNext([tcp_conn] { CHECK_DELETE_OBJ(tcp_conn); },
        "WsConnection::onTcpDisconnected, delete tcp_conn");
}

void WsConnection::onTcpSendCompleted()
{
    if (send_complete_cb_) {
        ++cb_level_;
        send_complete_cb_();
        --cb_level_;
    }
}

}
}
}
