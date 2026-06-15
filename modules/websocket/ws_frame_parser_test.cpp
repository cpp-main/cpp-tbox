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

#include "ws_frame_parser.h"

namespace tbox {
namespace websocket {

TEST(WsFrameParser, UnmaskedTextFrame)
{
    //! RFC 6455 Section 5.7 示例：单帧无掩码文本 "Hello"
    //! 0x81 0x05 0x48 0x65 0x6c 0x6c 0x6f
    uint8_t data[] = {0x81, 0x05, 'H', 'e', 'l', 'l', 'o'};
    WsFrameParser parser;
    size_t consumed = parser.parse(data, sizeof(data));

    EXPECT_EQ(sizeof(data), consumed);
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_TRUE(frame->fin);
    EXPECT_EQ(WsFrame::OpCode::kText, frame->opcode);
    EXPECT_EQ("Hello", frame->payload);
    delete frame;
}

TEST(WsFrameParser, MaskedTextFrame)
{
    //! RFC 6455 Section 5.7 示例：单帧有掩码文本 "Hello"
    //! 0x81 0x85 0x37 0xfa 0x21 0x3d 0x7f 0x9f 0x4d 0x51 0x58
    uint8_t data[] = {0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58};
    WsFrameParser parser;
    size_t consumed = parser.parse(data, sizeof(data));

    EXPECT_EQ(sizeof(data), consumed);
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_TRUE(frame->fin);
    EXPECT_EQ(WsFrame::OpCode::kText, frame->opcode);
    EXPECT_EQ("Hello", frame->payload);
    delete frame;
}

TEST(WsFrameParser, PingFrame)
{
    //! Ping 帧，无掩码
    uint8_t data[] = {0x89, 0x05, 'H', 'e', 'l', 'l', 'o'};
    WsFrameParser parser;
    parser.parse(data, sizeof(data));

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_EQ(WsFrame::OpCode::kPing, frame->opcode);
    EXPECT_EQ("Hello", frame->payload);
    delete frame;
}

TEST(WsFrameParser, CloseFrame)
{
    //! Close 帧，无掩码，code=1000
    uint8_t data[] = {0x88, 0x02, 0x03, 0xe8};
    WsFrameParser parser;
    parser.parse(data, sizeof(data));

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_EQ(WsFrame::OpCode::kClose, frame->opcode);
    EXPECT_EQ(1000, frame->closeCode());
    delete frame;
}

TEST(WsFrameParser, ExtendedPayloadLen16)
{
    //! 126 表示 16 位扩展长度
    //! 创建一个 payload_len = 200 的二进制帧
    std::vector<uint8_t> frame_data;
    frame_data.push_back(0x82); //! FIN + binary
    frame_data.push_back(126);  //! 16-bit extended length
    frame_data.push_back(0x00); //! 高字节: 200 >> 8 = 0
    frame_data.push_back(0xC8); //! 低字节: 200 & 0xFF = 200

    //! 200 字节的 payload (全是 0xAA)
    for (int i = 0; i < 200; ++i)
        frame_data.push_back(0xAA);

    WsFrameParser parser;
    size_t consumed = parser.parse(frame_data.data(), frame_data.size());

    EXPECT_EQ(frame_data.size(), consumed);
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_EQ(WsFrame::OpCode::kBinary, frame->opcode);
    EXPECT_EQ(200u, frame->payload.size());
    delete frame;
}

TEST(WsFrameParser, IncrementalParse)
{
    //! 测试增量解析：数据分两次喂入
    uint8_t data[] = {0x81, 0x05, 'H', 'e', 'l', 'l', 'o'};

    WsFrameParser parser;

    //! 第一次只喂2字节
    size_t consumed1 = parser.parse(data, 2);
    EXPECT_EQ(2, consumed1);
    EXPECT_EQ(WsFrameParser::State::kPayload, parser.state());

    //! 第二次喂剩余5字节
    size_t consumed2 = parser.parse(data + 2, 5);
    EXPECT_EQ(5, consumed2);
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_EQ("Hello", frame->payload);
    delete frame;
}

}
}
