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
#ifndef TBOX_WS_FRAME_H_20260612
#define TBOX_WS_FRAME_H_20260612

#include <cstdint>
#include <string>
#include <vector>

namespace tbox {
namespace websocket {

//! WebSocket 帧（RFC 6455）
struct WsFrame {
    //! 操作码
    enum class OpCode : uint8_t {
        kContinue = 0x0,   //!< 继续
        kText     = 0x1,   //!< 文本
        kBinary   = 0x2,   //!< 二进制
        kClose    = 0x8,   //!< 关闭连接
        kPing     = 0x9,   //!< Ping
        kPong     = 0xA,   //!< Pong
    };

    OpCode  opcode = OpCode::kContinue;
    bool    fin    = true;         //!< 是否为最后一帧
    std::string payload;           //!< 负载数据

    //! 是否为控制帧（Close/Ping/Pong）
    bool isControlFrame() const
    {
        return opcode == OpCode::kClose
            || opcode == OpCode::kPing
            || opcode == OpCode::kPong;
    }

    //! 从 Close 帧中提取关闭码和原因
    uint16_t closeCode() const
    {
        if (opcode != OpCode::kClose || payload.size() < 2)
            return 0;
        return (static_cast<uint16_t>(static_cast<uint8_t>(payload[0])) << 8)
              | static_cast<uint16_t>(static_cast<uint8_t>(payload[1]));
    }

    std::string closeReason() const
    {
        if (opcode != OpCode::kClose || payload.size() <= 2)
            return "";
        return payload.substr(2);
    }
};

}
}

#endif //TBOX_WS_FRAME_H_20260612
