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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <random>
#include <ctime>

#include <gtest/gtest.h>
#include "scalable_integer.h"

namespace tbox {
namespace util {

TEST(ScalableInteger, Dump_0) {
    uint8_t out_array[10];
    memset(out_array, 0xcc, sizeof(out_array));

    EXPECT_EQ(DumpScalableInteger(0, out_array, sizeof(out_array)), 1);
    EXPECT_EQ(out_array[0], 0);
    EXPECT_EQ(out_array[1], 0xcc);
}

TEST(ScalableInteger, Dump_127) {
    uint8_t out_array[10];
    memset(out_array, 0xcc, sizeof(out_array));

    EXPECT_EQ(DumpScalableInteger(127, out_array, sizeof(out_array)), 1);
    EXPECT_EQ(out_array[0], 127);
    EXPECT_EQ(out_array[1], 0xcc);
}

TEST(ScalableInteger, Dump_128) {
    uint8_t out_array[10];
    memset(out_array, 0xcc, sizeof(out_array));

    EXPECT_EQ(DumpScalableInteger(128, out_array, sizeof(out_array)), 2);
    EXPECT_EQ(out_array[0], 0x80);
    EXPECT_EQ(out_array[1], 0x00);
}

TEST(ScalableInteger, Dump_16511) {
    uint8_t out_array[10];
    memset(out_array, 0xcc, sizeof(out_array));

    EXPECT_EQ(DumpScalableInteger(16511, out_array, sizeof(out_array)), 2);
    EXPECT_EQ(out_array[0], 0xff);
    EXPECT_EQ(out_array[1], 0x7f);
}

TEST(ScalableInteger, Dump_16512) {
    uint8_t out_array[10];
    memset(out_array, 0xcc, sizeof(out_array));

    EXPECT_EQ(DumpScalableInteger(16512, out_array, sizeof(out_array)), 3);
    EXPECT_EQ(out_array[0], 0x80);
    EXPECT_EQ(out_array[1], 0x80);
    EXPECT_EQ(out_array[2], 0x00);
}

TEST(ScalableInteger, Parse_0) {
    uint8_t in_array[10];
    memset(in_array, 0xcc, sizeof(in_array));
    in_array[0] = 0;

    uint64_t value = 0xcc;
    EXPECT_EQ(ParseScalableInteger(in_array, sizeof(in_array), value), 1);
    EXPECT_EQ(value, 0);
}

TEST(ScalableInteger, Parse_127) {
    uint8_t in_array[10];
    memset(in_array, 0xcc, sizeof(in_array));
    in_array[0] = 0x7f;

    uint64_t value = 0xcc;
    EXPECT_EQ(ParseScalableInteger(in_array, sizeof(in_array), value), 1);
    EXPECT_EQ(value, 127);
}

TEST(ScalableInteger, Parse_128) {
    uint8_t in_array[10];
    memset(in_array, 0xcc, sizeof(in_array));
    in_array[0] = 0x80;
    in_array[1] = 0x00;

    uint64_t value = 0xcc;
    EXPECT_EQ(ParseScalableInteger(in_array, sizeof(in_array), value), 2);
    EXPECT_EQ(value, 128);
}

TEST(ScalableInteger, Parse_16511) {
    uint8_t in_array[10];
    memset(in_array, 0xcc, sizeof(in_array));
    in_array[0] = 0xff;
    in_array[1] = 0x7f;

    uint64_t value = 0xcc;
    EXPECT_EQ(ParseScalableInteger(in_array, sizeof(in_array), value), 2);
    EXPECT_EQ(value, 16511);
}

TEST(ScalableInteger, Parse_16512) {
    uint8_t in_array[10];
    memset(in_array, 0xcc, sizeof(in_array));
    in_array[0] = 0x80;
    in_array[1] = 0x80;
    in_array[2] = 0x00;

    uint64_t value = 0xcc;
    EXPECT_EQ(ParseScalableInteger(in_array, sizeof(in_array), value), 3);
    EXPECT_EQ(value, 16512);
}

TEST(ScalableInteger, DumpAndParseMax) {
    uint8_t array[10];

    memset(array, 0xcc, sizeof(array));
    uint64_t max_value = std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(DumpScalableInteger(max_value, array, sizeof(array)), 10);

    uint64_t value = 0;
    EXPECT_EQ(ParseScalableInteger(array, sizeof(array), value), 10);
    EXPECT_EQ(value, max_value);
}

//! 产生随机数，存入然后读出，对比
TEST(ScalableInteger, DumpAndParseRandom) {
    std::default_random_engine e;
    e.seed(time(nullptr));
    std::uniform_int_distribution<uint64_t> u(0, std::numeric_limits<uint64_t>::max());

    uint8_t array[10];

    for (int i = 0; i < 100000; ++i) {
        uint64_t write_value = u(e);
        EXPECT_NE(DumpScalableInteger(write_value, array, sizeof(array)), 0);

        uint64_t read_value = 0;
        EXPECT_NE(ParseScalableInteger(array, sizeof(array), read_value), 0);
        EXPECT_EQ(write_value, read_value);
    }
}

}
}
