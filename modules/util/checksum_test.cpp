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
#include "checksum.h"

namespace tbox {
namespace util {

TEST(Checksum, CalcCheckSum8) {
    uint8_t tmp_data[] = { 0x11, 0xff, 0xfe, 0xee, 0x34, 0xde, 0x00 };
    uint8_t crc8 = CalcCheckSum8(tmp_data, sizeof(tmp_data) - 1);
    tmp_data[sizeof(tmp_data) - 1] = crc8;

    EXPECT_EQ(CalcCheckSum8(tmp_data, sizeof(tmp_data)), 0);
}

TEST(Checksum, CalcCheckSum16_Even) {
    uint8_t tmp_data[] = { 0x11, 0xff, 0xfe, 0xee, 0x34, 0xde, 0x00, 0x00 };
    uint16_t crc16 = CalcCheckSum16(tmp_data, sizeof(tmp_data) - 2);
    tmp_data[sizeof(tmp_data) - 2] = crc16 >> 8;
    tmp_data[sizeof(tmp_data) - 1] = crc16 & 0xff;

    EXPECT_EQ(CalcCheckSum16(tmp_data, sizeof(tmp_data)), 0);
}

TEST(Checksum, CalcCheckSum16_Odd) {
    uint8_t tmp_data[] = { 0x11, 0xff, 0xfe, 0xee, 0x34, 0xde, 0x01, 0x00, 0x00, 0x00 };
    uint16_t crc16 = CalcCheckSum16(tmp_data, sizeof(tmp_data) - 3);
    tmp_data[sizeof(tmp_data) - 2] = crc16 >> 8;
    tmp_data[sizeof(tmp_data) - 1] = crc16 & 0xff;

    EXPECT_EQ(CalcCheckSum16(tmp_data, sizeof(tmp_data)), 0);
}

}
}
