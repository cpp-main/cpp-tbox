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
 * in the LICENSE file in the root of the project source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>

#include "ws_compressor.h"
#include "ws_frame_builder.h"
#include "ws_frame_parser.h"

namespace tbox {
namespace websocket {

//! === WsCompressor 测试 ===

TEST(WsCompressor, CompressDecompressRoundTrip)
{
    WsCompressionConfig config;
    config.enabled = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    //! 压缩 → 解压 → 验证
    std::string original = "Hello, WebSocket permessage-deflate! This is a test message.";
    std::string compressed = compressor.compress(original);
    ASSERT_FALSE(compressed.empty());

    //! 压缩后应该比原数据短（对重复性文本）
    EXPECT_LT(compressed.size(), original.size());

    std::string decompressed = compressor.decompress(compressed);
    EXPECT_EQ(original, decompressed);
}

TEST(WsCompressor, CompressDecompressLargeData)
{
    WsCompressionConfig config;
    config.enabled = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    //! 大数据测试
    std::string original(10000, 'A');
    std::string compressed = compressor.compress(original);
    ASSERT_FALSE(compressed.empty());
    EXPECT_LT(compressed.size(), original.size());

    std::string decompressed = compressor.decompress(compressed);
    EXPECT_EQ(original, decompressed);
}

TEST(WsCompressor, CompressDecompressEmptyData)
{
    WsCompressionConfig config;
    config.enabled = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    //! 空数据：压缩应该返回空（或不压缩的数据）
    std::string original = "";
    std::string compressed = compressor.compress(original);
    //! 空数据压缩后可能有少量数据（zlib 头信息）
    //! 但解压后应恢复为空
    if (!compressed.empty()) {
        std::string decompressed = compressor.decompress(compressed);
        EXPECT_EQ(original, decompressed);
    }
}

TEST(WsCompressor, NoContextTakeover)
{
    WsCompressionConfig config;
    config.enabled = true;
    config.no_context_takeover = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    //! 连续压缩多条不同消息，每条独立
    std::string msg1 = "First message with some repeated words words words";
    std::string msg2 = "Second message with different content xyz xyz xyz";
    std::string msg3 = "Third message 1234567890";

    std::string c1 = compressor.compress(msg1);
    std::string c2 = compressor.compress(msg2);
    std::string c3 = compressor.compress(msg3);

    ASSERT_FALSE(c1.empty());
    ASSERT_FALSE(c2.empty());
    ASSERT_FALSE(c3.empty());

    EXPECT_EQ(msg1, compressor.decompress(c1));
    EXPECT_EQ(msg2, compressor.decompress(c2));
    EXPECT_EQ(msg3, compressor.decompress(c3));
}

TEST(WsCompressor, DisabledCompression)
{
    WsCompressionConfig config;
    config.enabled = false;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    //! 禁用时 compress/decompress 返回空
    std::string data = "test data";
    EXPECT_EQ("", compressor.compress(data));
    EXPECT_EQ("", compressor.decompress(data));
}

//! === RSV1 帧解析测试 ===

TEST(WsFrameParser, Rsv1CompressedTextFrame)
{
    //! RSV1=1 的文本帧（压缩帧首帧）
    //! 第1字节: FIN=1, RSV1=1, opcode=0x01(text) = 0xC1
    //! 第2字节: MASK=0, len=5 = 0x05
    //! Payload: 5 字节压缩数据
    uint8_t data[] = {0xC1, 0x05, 0x48, 0x65, 0x6C, 0x6C, 0x6F};
    WsFrameParser parser;
    size_t consumed = parser.parse(data, sizeof(data));

    EXPECT_EQ(sizeof(data), consumed);
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *frame = parser.getFrame();
    ASSERT_NE(frame, nullptr);
    EXPECT_TRUE(frame->fin);
    EXPECT_TRUE(frame->rsv1);      //! RSV1 应为 true
    EXPECT_EQ(WsFrame::OpCode::kText, frame->opcode);
    delete frame;
}

TEST(WsFrameParser, Rsv2Rejected)
{
    //! RSV2=1 的帧应报错（目前不支持 RSV2）
    //! 第1字节: FIN=1, RSV2=1, opcode=0x01 = 0xA1
    uint8_t data[] = {0xA1, 0x05, 'H', 'e', 'l', 'l', 'o'};
    WsFrameParser parser;
    parser.parse(data, sizeof(data));

    EXPECT_EQ(WsFrameParser::State::kError, parser.state());
}

//! === RSV1 帧构建测试 ===

TEST(WsFrameBuilder, CompressedTextFrameServer)
{
    //! 服务端压缩帧：RSV1=1，不掩码
    auto frame = WsFrameBuilder::BuildFrame(WsFrame::OpCode::kText, true, "Hello", 5, true);
    EXPECT_EQ(0xC1, frame[0]);  //! FIN + RSV1 + text opcode
}

TEST(WsFrameBuilder, CompressedTextFrameClient)
{
    //! 客户端压缩帧：RSV1=1，掩码
    uint8_t mask_key[4] = {0x37, 0xfa, 0x21, 0x3d};
    auto frame = WsFrameBuilder::BuildMaskedFrame(WsFrame::OpCode::kText, true, "Hello", 5, mask_key, true);
    //! 第1字节: FIN + RSV1 + text opcode = 0xC1
    EXPECT_EQ(0xC1, frame[0]);
    //! 第2字节: MASK=1 + len=5 = 0x85
    EXPECT_EQ(0x85, frame[1]);
}

//! === 压缩帧 roundtrip 测试 ===

TEST(WsCompressor, CompressedFrameRoundTripServer)
{
    //! 模拟服务端发送压缩帧 → 客户端解析并解压
    WsCompressionConfig config;
    config.enabled = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    std::string original = "Hello WebSocket compression!";
    std::string compressed = compressor.compress(original);
    ASSERT_FALSE(compressed.empty());

    //! 服务端构建 RSV1=1 的帧（不掩码）
    auto frame = WsFrameBuilder::BuildFrame(WsFrame::OpCode::kText, true, compressed.data(), compressed.size(), true);

    //! 客户端解析帧
    WsFrameParser parser;
    parser.parse(frame.data(), frame.size());
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *parsed = parser.getFrame();
    ASSERT_NE(parsed, nullptr);
    EXPECT_TRUE(parsed->rsv1);
    EXPECT_EQ(WsFrame::OpCode::kText, parsed->opcode);

    //! 解压 payload
    std::string decompressed = compressor.decompress(parsed->payload);
    EXPECT_EQ(original, decompressed);
    delete parsed;
}

TEST(WsCompressor, CompressedFrameRoundTripClient)
{
    //! 模拟客户端发送压缩帧 → 服务端解析并解压
    WsCompressionConfig config;
    config.enabled = true;

    WsCompressor compressor;
    ASSERT_TRUE(compressor.initialize(config));

    std::string original = "Client compressed message!";
    std::string compressed = compressor.compress(original);
    ASSERT_FALSE(compressed.empty());

    //! 客户端构建 RSV1=1 的掩码帧
    auto frame = WsFrameBuilder::BuildMaskedFrame(WsFrame::OpCode::kText, true,
                                                  compressed.data(), compressed.size(),
                                                  nullptr, true);
    //! 服务端解析帧
    WsFrameParser parser;
    parser.parse(frame.data(), frame.size());
    EXPECT_EQ(WsFrameParser::State::kFinished, parser.state());

    WsFrame *parsed = parser.getFrame();
    ASSERT_NE(parsed, nullptr);
    EXPECT_TRUE(parsed->rsv1);
    EXPECT_EQ(WsFrame::OpCode::kText, parsed->opcode);

    //! 解压 payload
    std::string decompressed = compressor.decompress(parsed->payload);
    EXPECT_EQ(original, decompressed);
    delete parsed;
}

}
}
