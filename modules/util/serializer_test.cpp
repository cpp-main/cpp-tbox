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
#include "serializer.h"

using namespace tbox::util;

TEST(Serializer, big_endian_raw) {
    uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22
    };
    uint8_t data_be_write[17] = {0x00};

    Serializer w(data_be_write, sizeof(data_be_write));
    w.setEndian(Endian::kBig);

    EXPECT_TRUE(w.append(uint8_t(1)));
    EXPECT_TRUE(w.append(uint16_t(0x0203)));
    EXPECT_TRUE(w.append(uint32_t(0x04050607)));
    EXPECT_TRUE(w.append(uint64_t(0x1112131415161718ull)));
    uint8_t tmp[2] = {0x21, 0x22};
    EXPECT_TRUE(w.append(tmp, 2));
    EXPECT_FALSE(w.append(uint8_t(1)));

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write, sizeof(data_be_write)));
}

TEST(Serializer, big_endian_vector) {
    uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22
    };

    std::vector<uint8_t> data_be_write;
    Serializer w(data_be_write);
    w.setEndian(Endian::kBig);

    EXPECT_TRUE(w.append(uint8_t(1)));
    EXPECT_TRUE(w.append(uint16_t(0x0203)));
    EXPECT_TRUE(w.append(uint32_t(0x04050607)));
    EXPECT_TRUE(w.append(uint64_t(0x1112131415161718ull)));
    uint8_t tmp[2] = {0x21, 0x22};
    EXPECT_TRUE(w.append(tmp, 2));
    EXPECT_EQ(data_be_write.size(), 17u);

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write.data(), sizeof(data_to_be)));
}

TEST(Serializer, big_endian_stream) {
    const uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    };
    uint8_t data_be_write[sizeof(data_to_be)] = {0x00};

    Serializer w(data_be_write, sizeof(data_be_write));
    w << Endian::kBig << uint8_t(1) << uint16_t(0x0203) << uint32_t(0x04050607) << uint64_t(0x1112131415161718ull);

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write, sizeof(data_to_be)));
}

TEST(Serializer, little_endian) {
    const uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22
    };
    uint8_t data_be_write[sizeof(data_to_be)] = {0x00};

    Serializer w(data_be_write, sizeof(data_be_write));
    w.setEndian(Endian::kLittle);

    EXPECT_TRUE(w.append(uint8_t(1)));
    EXPECT_TRUE(w.append(uint16_t(0x0302)));
    EXPECT_TRUE(w.append(uint32_t(0x07060504)));
    EXPECT_TRUE(w.append(uint64_t(0x1817161514131211ull)));
    uint8_t tmp[2] = {0x21, 0x22};
    EXPECT_TRUE(w.append(tmp, 2));
    EXPECT_FALSE(w.append(uint8_t(1)));

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write, sizeof(data_to_be)));
}

TEST(Serializer, little_endian_stream) {
    uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    };
    uint8_t data_be_write[sizeof(data_to_be)] = {0x00};

    Serializer w(data_be_write, sizeof(data_be_write));
    w << Endian::kLittle << uint8_t(1) << uint16_t(0x0302) << uint32_t(0x07060504) << uint64_t(0x1817161514131211ull);

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write, sizeof(data_to_be)));
}

TEST(Deserializer, big_endian) {
    uint8_t data_be_read[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22, 0x23, 0x24
    };

    Deserializer r(data_be_read, sizeof(data_be_read));
    r.setEndian(Endian::kBig);

    uint8_t u8;
    EXPECT_TRUE(r.fetch(u8));
    EXPECT_EQ(u8, 0x1);

    uint16_t u16;
    EXPECT_TRUE(r.fetch(u16));
    EXPECT_EQ(u16, 0x0203);

    uint32_t u32;
    EXPECT_TRUE(r.fetch(u32));
    EXPECT_EQ(u32, 0x04050607ul);

    uint64_t u64;
    EXPECT_TRUE(r.fetch(u64));
    EXPECT_EQ(u64, 0x1112131415161718ull);

    uint8_t read_out[2];
    EXPECT_TRUE(r.fetch(read_out, 2));
    EXPECT_EQ(read_out[0], 0x21);
    EXPECT_EQ(read_out[1], 0x22);

    EXPECT_FALSE(r.fetch(u32));
}

TEST(Deserializer, big_endian_stream) {
    uint8_t data_be_read[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    };

    Deserializer r(data_be_read, sizeof(data_be_read));

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    r >> Endian::kBig >> u8 >> u16 >> u32 >> u64;

    EXPECT_EQ(u8, 0x1);
    EXPECT_EQ(u16, 0x0203);
    EXPECT_EQ(u32, 0x04050607ul);
    EXPECT_EQ(u64, 0x1112131415161718ull);
}

TEST(Deserializer, little_endian) {
    uint8_t data_be_read[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22, 0x23, 0x24
    };

    Deserializer r(data_be_read, sizeof(data_be_read));
    r.setEndian(Endian::kLittle);

    uint8_t u8;
    EXPECT_TRUE(r.fetch(u8));
    EXPECT_EQ(u8, 0x1);

    uint16_t u16;
    EXPECT_TRUE(r.fetch(u16));
    EXPECT_EQ(u16, 0x0302);

    uint32_t u32;
    EXPECT_TRUE(r.fetch(u32));
    EXPECT_EQ(u32, 0x07060504ul);

    uint64_t u64;
    EXPECT_TRUE(r.fetch(u64));
    EXPECT_EQ(u64, 0x1817161514131211ull);

    uint8_t read_out[2];
    EXPECT_TRUE(r.fetch(read_out, 2));
    EXPECT_EQ(read_out[0], 0x21);
    EXPECT_EQ(read_out[1], 0x22);

    EXPECT_FALSE(r.fetch(u32));
}

TEST(Deserializer, little_endian_stream) {
    uint8_t data_be_read[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    };

    Deserializer r(data_be_read, sizeof(data_be_read));

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    r >> Endian::kLittle >> u8 >> u16 >> u32 >> u64;

    EXPECT_EQ(u8, 0x1);
    EXPECT_EQ(u16, 0x0302);
    EXPECT_EQ(u32, 0x07060504ul);
    EXPECT_EQ(u64, 0x1817161514131211ull);
}

TEST(SerializerDeserialize, big_endian)
{
    uint8_t buff[50];
    Serializer s(buff, sizeof(buff), Endian::kBig);
    s << int8_t(-1) << uint8_t(2) << int16_t(-3) << uint16_t(4) << int32_t(-5) << uint32_t(6)
        << int64_t(-7) << uint64_t(8) << float(1.0) << double(12.3);

    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    float ff;
    double dd;

    Deserializer d(buff, s.pos(), Endian::kBig);
    d >> i8 >> u8 >> i16 >> u16 >> i32 >> u32 >> i64 >> u64 >> ff >> dd;

    EXPECT_EQ(i8, -1);
    EXPECT_EQ(u8, 2);
    EXPECT_EQ(i16, -3);
    EXPECT_EQ(u16, 4);
    EXPECT_EQ(i32, -5);
    EXPECT_EQ(u32, 6);
    EXPECT_EQ(i64, -7);
    EXPECT_EQ(u64, 8);
    EXPECT_FLOAT_EQ(ff, 1.0);
    EXPECT_DOUBLE_EQ(dd, 12.3);
}

TEST(SerializerDeserialize, little_endian)
{
    uint8_t buff[50];
    Serializer s(buff, sizeof(buff), Endian::kLittle);
    s << int8_t(-1) << uint8_t(2) << int16_t(-3) << uint16_t(4) << int32_t(-5) << uint32_t(6)
        << int64_t(-7) << uint64_t(8) << float(1.0) << double(12.3);

    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    float ff;
    double dd;

    Deserializer d(buff, s.pos(), Endian::kLittle);
    d >> i8 >> u8 >> i16 >> u16 >> i32 >> u32 >> i64 >> u64 >> ff >> dd;

    EXPECT_EQ(i8, -1);
    EXPECT_EQ(u8, 2);
    EXPECT_EQ(i16, -3);
    EXPECT_EQ(u16, 4);
    EXPECT_EQ(i32, -5);
    EXPECT_EQ(u32, 6);
    EXPECT_EQ(i64, -7);
    EXPECT_EQ(u64, 8);
    EXPECT_FLOAT_EQ(ff, 1.0);
    EXPECT_DOUBLE_EQ(dd, 12.3);
}

