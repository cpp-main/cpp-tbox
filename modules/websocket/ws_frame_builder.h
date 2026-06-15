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
//! 用于服务端发送帧（服务端帧不使用掩码）
class WsFrameBuilder {
  public:
    //! 构建文本帧
    static std::vector<uint8_t> BuildTextFrame(const std::string &text);

    //! 构建二进制帧
    static std::vector<uint8_t> BuildBinaryFrame(const void *data, size_t len);
    static std::vector<uint8_t> BuildBinaryFrame(const std::vector<uint8_t> &data);

    //! 构建关闭帧
    static std::vector<uint8_t> BuildCloseFrame(uint16_t code = 1000, const std::string &reason = "");

    //! 构建 Ping 帧
    static std::vector<uint8_t> BuildPingFrame(const std::string &data = "");

    //! 构建 Pong 帧
    static std::vector<uint8_t> BuildPongFrame(const std::string &data = "");

    //! 通用帧构建
    static std::vector<uint8_t> BuildFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len);
};

}
}

#endif //TBOX_WS_FRAME_BUILDER_H_20260612
