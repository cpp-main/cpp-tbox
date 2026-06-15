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
#include "ws_frame_builder.h"

#include <cstring>

namespace tbox {
namespace websocket {

std::vector<uint8_t> WsFrameBuilder::BuildTextFrame(const std::string &text)
{
    return BuildFrame(WsFrame::OpCode::kText, true, text.data(), text.size());
}

std::vector<uint8_t> WsFrameBuilder::BuildBinaryFrame(const void *data, size_t len)
{
    return BuildFrame(WsFrame::OpCode::kBinary, true, data, len);
}

std::vector<uint8_t> WsFrameBuilder::BuildBinaryFrame(const std::vector<uint8_t> &data)
{
    return BuildFrame(WsFrame::OpCode::kBinary, true, data.data(), data.size());
}

std::vector<uint8_t> WsFrameBuilder::BuildCloseFrame(uint16_t code, const std::string &reason)
{
    //! Close 帧的 payload: 2字节关闭码(大端序) + 可选的原因字符串
    std::vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(code & 0xFF));
    if (!reason.empty())
        payload.insert(payload.end(), reason.begin(), reason.end());

    return BuildFrame(WsFrame::OpCode::kClose, true, payload.data(), payload.size());
}

std::vector<uint8_t> WsFrameBuilder::BuildPingFrame(const std::string &data)
{
    return BuildFrame(WsFrame::OpCode::kPing, true, data.data(), data.size());
}

std::vector<uint8_t> WsFrameBuilder::BuildPongFrame(const std::string &data)
{
    return BuildFrame(WsFrame::OpCode::kPong, true, data.data(), data.size());
}

std::vector<uint8_t> WsFrameBuilder::BuildFrame(WsFrame::OpCode opcode, bool fin, const void *payload_ptr, size_t payload_len)
{
    std::vector<uint8_t> frame;

    //! 第1字节：FIN + RSV1-3(0) + Opcode
    uint8_t byte0 = static_cast<uint8_t>(opcode);
    if (fin)
        byte0 |= 0x80;
    frame.push_back(byte0);

    //! 第2字节：MASK=0(服务端不掩码) + Payload length
    //! 服务端发送的帧不使用掩码（RFC 6455 Section 5.3）
    if (payload_len < 125) {
        frame.push_back(static_cast<uint8_t>(payload_len));
    } else if (payload_len <= 65535) {
        frame.push_back(126);
        frame.push_back(static_cast<uint8_t>((payload_len >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(payload_len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i)
            frame.push_back(static_cast<uint8_t>((payload_len >> (i * 8)) & 0xFF));
    }

    //! Payload 数据
    if (payload_ptr != nullptr && payload_len > 0) {
        const uint8_t *p = static_cast<const uint8_t*>(payload_ptr);
        frame.insert(frame.end(), p, p + payload_len);
    }

    return frame;
}

}
}
