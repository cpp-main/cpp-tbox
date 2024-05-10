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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include "base64.h"

namespace tbox {
namespace util {
namespace base64 {

TEST(Base64, EncodeLength) {
    EXPECT_EQ(EncodeLength(0), 0);
    EXPECT_EQ(EncodeLength(1), 4);
    EXPECT_EQ(EncodeLength(2), 4);
    EXPECT_EQ(EncodeLength(3), 4);
    EXPECT_EQ(EncodeLength(4), 8);
    EXPECT_EQ(EncodeLength(5), 8);
}

TEST(Base64, DecodeLength) {
    EXPECT_EQ(DecodeLength("MQ"), 0);
    EXPECT_EQ(DecodeLength("MQ="), 0);
    EXPECT_EQ(DecodeLength("MQ=="), 1);
    EXPECT_EQ(DecodeLength("MTI="), 2);
    EXPECT_EQ(DecodeLength("MTIz"), 3);
    EXPECT_EQ(DecodeLength("MTIzNA=="), 4);
}

TEST(Base64, Encode) {
    const char* in = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char out[37] = {0};
    EXPECT_EQ(Encode(in, 26, out, 36), 36);
    EXPECT_STREQ(out, "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=");
}

TEST(Base64, Encode_ReturnString) {
    const char* in = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    EXPECT_EQ(Encode(in, 26), "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=");
}

TEST(Base64, Encode_LenthNotEnough) {
    const char* in = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char out[36] = {0};
    EXPECT_EQ(Encode(in, 26, out, 35), 0);
}

TEST(Base64, Decode) {
    const char* in = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=";
    char out[27] = {0};
    EXPECT_EQ(Decode(in, 36, out, 26), 26);
    EXPECT_STREQ(out, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

TEST(Base64, DecodeText) {
    const char* in = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=";
    char out[27] = {0};
    EXPECT_EQ(Decode(in, out, 26), 26);
    EXPECT_STREQ(out, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

TEST(Base64, DecodeStringToVector) {
    const std::string in = "aGVsbG8h";
    std::vector<uint8_t> out;
    EXPECT_EQ(Decode(in, out), 6);
    EXPECT_EQ(out, std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o', '!'}));
}

TEST(Base64, DecodeStringToVector_Pad1) {
    const std::string in = "aGVsbG8hIQ==";
    std::vector<uint8_t> out;
    EXPECT_EQ(Decode(in, out), 7);
    EXPECT_EQ(out, std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o', '!', '!'}));
}

TEST(Base64, DecodeStringToVector_Pad2) {
    const std::string in = "aGVsbG8hISE=";
    std::vector<uint8_t> out;
    EXPECT_EQ(Decode(in, out), 8);
    EXPECT_EQ(out, std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o', '!', '!', '!'}));
}

TEST(Base64, DecodeStringToVector_Pad3) {
    const std::string in = "aGVsbG8=";
    std::vector<uint8_t> out;
    EXPECT_EQ(Decode(in, out), 5);
    EXPECT_EQ(out, std::vector<uint8_t>({'h', 'e', 'l', 'l', 'o'}));
}

TEST(Base64, DecodeStringToVectorFail) {
    const std::string in = "";
    std::vector<uint8_t> out;
    EXPECT_FALSE(Decode(in, out));
}

TEST(Base64, Decode_LenthNotEnough) {
    const char* in = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=";
    char out[27] = {0};
    EXPECT_EQ(Decode(in, 36, out, 25), 0);
}

}
}
}
