#include <gtest/gtest.h>
#include "serializer.h"

using namespace tbox::util;

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

TEST(Deserializer, litte_endian) {
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

TEST(Serializer, big_endian) {
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

TEST(Serializer, litte_endian) {
    uint8_t data_to_be[] = {
        0x01,
        0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x21, 0x22
    };
    uint8_t data_be_write[17] = {0x00};

    Serializer w(data_be_write, sizeof(data_be_write));
    w.setEndian(Endian::kLittle);

    EXPECT_TRUE(w.append(uint8_t(1)));
    EXPECT_TRUE(w.append(uint16_t(0x0302)));
    EXPECT_TRUE(w.append(uint32_t(0x07060504)));
    EXPECT_TRUE(w.append(uint64_t(0x1817161514131211ull)));
    uint8_t tmp[2] = {0x21, 0x22};
    EXPECT_TRUE(w.append(tmp, 2));
    EXPECT_FALSE(w.append(uint8_t(1)));

    EXPECT_EQ(0, memcmp(data_to_be, data_be_write, sizeof(data_be_write)));
}
