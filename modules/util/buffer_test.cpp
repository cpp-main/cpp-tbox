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

//! 放开权限，方便测试
#define protected public
#define private public

#include "buffer.h"

namespace tbox {
namespace util {

TEST(Buffer, constuct) {
    Buffer b1;
    EXPECT_EQ(b1.readableSize(), 0u);

    Buffer b2(b1);
    EXPECT_EQ(b2.readableSize(), 0u);
}

TEST(Buffer, append_and_fetch) {
    Buffer b1(0);
    //! 先插入4个字节
    b1.append("abcd", 4);
    EXPECT_EQ(b1.readableSize(), 4u);
    //! 再插入4个字节
    b1.append("efgh", 4);
    EXPECT_EQ(b1.readableSize(), 8u);
    //! 将所有的数据读出来
    char buff1[10] = { 0 };
    EXPECT_EQ(b1.fetch(buff1, 10), 8u);
    EXPECT_STREQ(buff1, "abcdefgh");
    //! 现在缓冲区空了
    EXPECT_EQ(b1.readableSize(), 0u);
}

/**
 * 设置缓冲初始长度为8，先写入7字节，再读走5字节，再写入5字节
 * 检查缓冲区长度是否是8字节。
 * 本测试的目的是观察在写入时缓冲区长度不够时，Buffer会不会移动可读取的数据
 */
TEST(Buffer, append_and_move_data) {
    Buffer b1(8);
    b1.append("1234567", 7);    //! 插入后只剩1字节
    b1.hasRead(5);              //! 读走5字节
    b1.append("89abc", 5);      //! 插入5字节
    EXPECT_EQ(b1.buffer_size_, 8u);
    char buff[10] = { 0 };
    EXPECT_EQ(b1.fetch(buff, 10), 7u);
    EXPECT_STREQ(buff, "6789abc");
    EXPECT_EQ(b1.readableSize(), 0u);
}

/**
 * 检查通过readBegin()读取数据是否正常
 */
TEST(Buffer, readBegin_hasRead) {
    Buffer b(8);
    const char *str = "hello world, my name is Sid Lee";
    size_t str_size = strlen(str) + 1;
    b.append(str, str_size);
    EXPECT_EQ(b.readableSize(), str_size);
    EXPECT_STREQ((const char*)b.readableBegin(), str);
    b.hasRead(14);
    EXPECT_STREQ((const char*)b.readableBegin(), (str + 14));

    b.hasRead(500);
    EXPECT_EQ(b.readableSize(), 0u);
}

/**
 * 检查通过writeBegin()写数据是否正常
 */
TEST(Buffer, writeBegin_hasWriten) {
    Buffer b(0);
    const char *str = "hello world";
    size_t str_size = strlen(str) + 1;
    b.ensureWritableSize(str_size);
    strcpy((char*)b.writableBegin(), str);
    b.hasWritten(str_size);

    EXPECT_STREQ((const char*)b.readableBegin(), str);
}

TEST(Buffer, swap) {
    Buffer b1(8);
    b1.append("abc", 4);

    Buffer b2(2);
    b2.append("hello", 6);

    b1.swap(b2);
    EXPECT_EQ(b1.readableSize(), 6u);
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b1.readableBegin(), "hello");
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(Buffer, reset) {
    Buffer b1(8);
    b1.append("abc", 4);
    b1.reset();

    EXPECT_EQ(b1.readableSize(), 0u);
    EXPECT_TRUE(b1.buffer_ptr_ == NULL);
}

TEST(Buffer, copy_construct) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(b1);
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(Buffer, move_construct) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(std::move(b1));
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");

    EXPECT_EQ(b1.readableBegin(), nullptr);
    EXPECT_EQ(b1.readableSize(), 0u);
}

TEST(Buffer, copy_assign) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(43);
    b2.append("1234567890", 10);
    b2 = b1;
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");
}

TEST(Buffer, move_assign) {
    Buffer b1(8);
    b1.append("abc", 4);
    Buffer b2(43);
    b2.append("1234567890", 10);
    b2 = std::move(b1);
    EXPECT_EQ(b2.readableSize(), 4u);
    EXPECT_STREQ((const char*)b2.readableBegin(), "abc");

    EXPECT_EQ(b1.readableBegin(), nullptr);
    EXPECT_EQ(b1.readableSize(), 0u);
}

TEST(Buffer, huge_data) {
    Buffer b;
    int orig_data[100] = { 0 };
    for (int i = 0; i < 100; ++i) {
        orig_data[i] = i;
    }

    //! 存入10000个数据
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(b.append(orig_data, sizeof(orig_data)), sizeof(orig_data));
    }

    EXPECT_EQ(b.readableSize(), (sizeof(orig_data) * 10000));
    size_t max_size = b.buffer_size_;

    //! 取走9000个数据
    for (int i = 0; i < 9000; ++i) {
        int read_data[100] = { 0 };
        EXPECT_EQ(b.fetch(read_data, sizeof(read_data)), sizeof(read_data));
        EXPECT_EQ(memcmp(read_data, orig_data, sizeof(read_data)), 0);
    }

    //! 再写入9000个数据
    for (int i = 0; i < 9000; ++i) {
        EXPECT_EQ(b.append(orig_data, sizeof(orig_data)), sizeof(orig_data));
    }

    //! 期望缓冲的空间大小没有变化
    EXPECT_EQ(max_size, b.buffer_size_);

    //! 取走10000个数据
    for (int i = 0; i < 10000; ++i) {
        int read_data[100] = { 0 };
        EXPECT_EQ(b.fetch(read_data, sizeof(read_data)), sizeof(read_data));
        EXPECT_EQ(memcmp(read_data, orig_data, sizeof(read_data)), 0);
    }

    //! 期望缓冲的空间大小没有变化
    EXPECT_EQ(max_size, b.buffer_size_);
    EXPECT_EQ(b.readableSize(), 0u);
}

TEST(Buffer, shrink_after_readsize_0) {
    Buffer b;
    for (int i = 0; i < 100; ++i)
        b.append("1234567890", 10);
    b.hasReadAll();
    b.shrink();
    EXPECT_EQ(b.buffer_ptr_, nullptr);
    //!使用valgrind检查内存是否存在泄漏
}

TEST(Buffer, shrink_after_readsize_not_0) {
    Buffer b;
    for (int i = 0; i < 100; ++i)
        b.append("1234567890", 10);
    b.hasRead(990);
    b.shrink();
    EXPECT_NE(b.buffer_ptr_, nullptr);
    EXPECT_EQ(b.readableSize(), 10);
    //!使用valgrind检查内存是否存在泄漏
}

TEST(Buffer, read_all_except_index_reset) {
    Buffer b;

    b.append("123456789", 10);
    char read_data[10] = { 0 };
    b.fetch(read_data, 10);

    EXPECT_EQ(b.read_index_, 0u);
    EXPECT_EQ(b.write_index_, 0u);
}

}
}
