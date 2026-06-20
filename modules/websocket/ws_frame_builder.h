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
#ifndef TBOX_WS_FRAME_BUILDER_H_20260612
#define TBOX_WS_FRAME_BUILDER_H_20260612

#include "ws_frame.h"
#include <cstdint>
#include <string>
#include <vector>

namespace tbox {
namespace websocket {

//! WebSocket 帧构建器（RFC 6455）
//! 服务端帧不使用掩码，客户端帧必须使用掩码
class WsFrameBuilder {
  public:
    //! === 服务端帧（不掩码） ===

    //! 构建文本帧（服务端）
    static std::vector<uint8_t> BuildTextFrame(const std::string &text);

    //! 构建二进制帧（服务端）
    static std::vector<uint8_t> BuildBinaryFrame(const void *data, size_t len);
    static std::vector<uint8_t> BuildBinaryFrame(const std::vector<uint8_t> &data);

    //! 构建关闭帧（服务端）
    static std::vector<uint8_t> BuildCloseFrame(uint16_t code = 1000, const std::string &reason = "");

    //! 构建 Ping 帧（服务端）
    static std::vector<uint8_t> BuildPingFrame(const std::string &data = "");

    //! 构建 Pong 帧（服务端）
    static std::vector<uint8_t> BuildPongFrame(const std::string &data = "");

    //! 通用帧构建（服务端，不掩码）
    static std::vector<uint8_t> BuildFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len);

    //! === 客户端帧（掩码） ===
    //! RFC 6455 Section 5.3：客户端发送的帧必须使用掩码

    //! 构建文本帧（客户端，掩码）
    static std::vector<uint8_t> BuildMaskedTextFrame(const std::string &text);

    //! 构建二进制帧（客户端，掩码）
    static std::vector<uint8_t> BuildMaskedBinaryFrame(const void *data, size_t len);
    static std::vector<uint8_t> BuildMaskedBinaryFrame(const std::vector<uint8_t> &data);

    //! 构建关闭帧（客户端，掩码）
    static std::vector<uint8_t> BuildMaskedCloseFrame(uint16_t code = 1000, const std::string &reason = "");

    //! 构建 Ping 帧（客户端，掩码）
    static std::vector<uint8_t> BuildMaskedPingFrame(const std::string &data = "");

    //! 构建 Pong 帧（客户端，掩码）
    static std::vector<uint8_t> BuildMaskedPongFrame(const std::string &data = "");

    //! 通用帧构建（客户端，掩码）
    //! mask_key 为 4 字节掩码密钥，若为 nullptr 则自动随机生成
    static std::vector<uint8_t> BuildMaskedFrame(WsFrame::OpCode opcode, bool fin,
                                                 const void *payload, size_t payload_len,
                                                 const uint8_t *mask_key = nullptr);
};

}
}

#endif //TBOX_WS_FRAME_BUILDER_H_20260612
