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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>

#include "ws_frame_builder.h"
#include "ws_frame_parser.h"

namespace tbox {
namespace websocket {

TEST(WsFrameBuilder, TextFrame)
{
    auto frame = WsFrameBuilder::BuildTextFrame("Hello");
    //! 期望: 0x81 0x05 'H' 'e' 'l' 'l' 'o'
    EXPECT_EQ(7u, frame.size());
    EXPECT_EQ(0x81, frame[0]);
    EXPECT_EQ(0x05, frame[1]);
    EXPECT_EQ('H', frame[2]);
}

TEST(WsFrameBuilder, BinaryFrame)
{
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto frame = WsFrameBuilder::BuildBinaryFrame(data);
    //! 期望: 0x82 0x03 0x01 0x02 0x03
    EXPECT_EQ(5u, frame.size());
    EXPECT_EQ(0x82, frame[0]);
    EXPECT_EQ(0x03, frame[1]);
}

TEST(WsFrameBuilder, CloseFrame)
{
    auto frame = WsFrameBuilder::BuildCloseFrame(1000, "normal");
    //! 期望: 0x88 + len + 0x03E8 + "normal"
    //! payload = 2 + 6 = 8 bytes
    EXPECT_EQ(0x88, frame[0]);
    EXPECT_EQ(8u, frame[1]);
    //! 关闭码: 0x03 0xE8 (1000 大端序)
    EXPECT_EQ(0x03, frame[2]);
    EXPECT_EQ(0xE8, frame[3]);
}

TEST(WsFrameBuilder, PingFrame)
{
    auto frame = WsFrameBuilder::BuildPingFrame("test");
    EXPECT_EQ(0x89, frame[0]);
    EXPECT_EQ(4u, frame[1]);
}

TEST(WsFrameBuilder, PongFrame)
{
    auto frame = WsFrameBuilder::BuildPongFrame("test");
    EXPECT_EQ(0x8A, frame[0]);
    EXPECT_EQ(4u, frame[1]);
}

TEST(WsFrameBuilder, LargePayload16)
{
    //! Payload 超过 125 字节，使用 16 位扩展长度
    std::string large_text(200, 'A');
    auto frame = WsFrameBuilder::BuildTextFrame(large_text);
    EXPECT_EQ(0x81, frame[0]);
    EXPECT_EQ(126, frame[1]); //! 16-bit extended length marker
    //! 长度 = 200 = 0x00C8
    EXPECT_EQ(0x00, frame[2]);
    EXPECT_EQ(0xC8, frame[3]);
    EXPECT_EQ(200u + 4u, frame.size()); //! 4 header bytes + 200 payload
}

TEST(WsFrameBuilder, RoundTrip)
{
    //! 构建 → 解析 → 验证
    auto built = WsFrameBuilder::BuildTextFrame("RoundTrip Test");

    WsFrameParser parser;
    parser.parse(built.data(), built.size());

    WsFrame *parsed = parser.getFrame();
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(WsFrame::OpCode::kText, parsed->opcode);
    EXPECT_TRUE(parsed->fin);
    EXPECT_EQ("RoundTrip Test", parsed->payload);
    delete parsed;
}

}
}
