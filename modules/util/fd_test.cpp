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
#include "fd.h"

#include <gtest/gtest.h>

namespace tbox {
namespace util {

TEST(Fd, close_func)
{
    int closed_fd = -1;
    {
        //! 期望它在退出的时候能自动执行对应close函数，并将closed_fd置为12
        Fd fd(12, [&](int v) { closed_fd = v; });
    }
    EXPECT_EQ(closed_fd, 12);
}

TEST(Fd, swap)
{
    Fd fd(12);
    ASSERT_EQ(fd.get(), 12);
    Fd other(13);
    fd.swap(other);
    EXPECT_EQ(fd.get(), 13);
    EXPECT_EQ(other.get(), 12);
}

TEST(Fd, move_1)
{
    Fd fd1(12);
    Fd fd2(13);
    fd1 = std::move(fd2);
    EXPECT_EQ(fd1.get(), 13);
    EXPECT_EQ(fd2.get(), -1);
}

TEST(Fd, move_2)
{
    Fd fd1(12);
    Fd fd2(std::move(fd1));
    EXPECT_EQ(fd2.get(), 12);
    EXPECT_EQ(fd1.get(), -1);
}

TEST(Fd, reset)
{
    Fd fd(12);
    fd.reset();
    EXPECT_EQ(fd.get(), -1);
}

TEST(Fd, close) {
    int close_times = 0;
    Fd fd1(12, [&](int v) { ++close_times; (void)v; });
    {
        Fd fd2 = fd1;
        {
            Fd fd3 = fd2;
            EXPECT_EQ(fd3.get(), 12);
            EXPECT_EQ(close_times, 0);
            fd3.close();
            EXPECT_EQ(close_times, 1);
        }
        EXPECT_EQ(fd2.get(), -1);
        EXPECT_TRUE(fd2.isNull());
        EXPECT_EQ(close_times, 1);
    }

    EXPECT_EQ(fd1.get(), -1);
    EXPECT_TRUE(fd1.isNull());
    EXPECT_EQ(close_times, 1);
}

TEST(Fd, cast)
{
    Fd fd(12);
    EXPECT_EQ(fd.get(), 12);
}

TEST(Fd, copy_construct)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2(fd1);    //! 执行了一个复制构造
        EXPECT_EQ(close_times, 0);
        EXPECT_EQ(fd2.get(), 12);
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 1);
}

TEST(Fd, copy_assign_no_value)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2;
        fd2 = fd1;  //! 执行了一次复制赋值
        EXPECT_EQ(close_times, 0);
        EXPECT_EQ(fd2.get(), 12);
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 1);
}

TEST(Fd, copy_assign_has_value)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2(13, [&](int v) { closed_fd = v; ++close_times; });
        fd2 = fd1;  //! 执行了一次复制赋值，13被释放了
        EXPECT_EQ(close_times, 1);
        EXPECT_EQ(closed_fd, 13);
        EXPECT_EQ(fd2.get(), 12);
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 2);
}

TEST(Fd, move_construct)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2(std::move(fd1)); //! 移动构造
        EXPECT_EQ(close_times, 0);
        EXPECT_EQ(fd2.get(), 12);
        EXPECT_TRUE(fd1.isNull());
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 1);
}

//! 将有值的fd1给到fd2
TEST(Fd, move_assign_1)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2;
        fd2 = std::move(fd1);
        EXPECT_EQ(close_times, 0);
        EXPECT_EQ(fd2.get(), 12);
        EXPECT_TRUE(fd1.isNull());
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 1);
}

//! 将无值的fd1给到有值的fd2
TEST(Fd, move_assign_2)
{
    int closed_fd = -1;
    int close_times = 0;

    Fd fd1;
    Fd fd2(12, [&](int v) { closed_fd = v; ++close_times; });
    fd2 = std::move(fd1);
    EXPECT_EQ(close_times, 1);
    EXPECT_EQ(closed_fd, 12);

    EXPECT_EQ(fd2.get(), -1);
    EXPECT_TRUE(fd1.isNull());
    EXPECT_TRUE(fd2.isNull());
}

//! fd1 与 fd2 都有值，将 fd2 给 fd1
TEST(Fd, move_assign_3)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1(12, [&](int v) { closed_fd = v; ++close_times; });
        Fd fd2(13, [&](int v) { closed_fd = v; ++close_times; });
        fd2 = std::move(fd1);
        EXPECT_EQ(close_times, 1);
        EXPECT_EQ(closed_fd, 13);
        EXPECT_EQ(fd2.get(), 12);
        EXPECT_TRUE(fd1.isNull());
    }
    EXPECT_EQ(closed_fd, 12);
    EXPECT_EQ(close_times, 2);
}

//! fd无值fd2有值，将 fd2 给 fd1
TEST(Fd, move_assign_4)
{
    int closed_fd = -1;
    int close_times = 0;
    {
        Fd fd1;
        Fd fd2(13, [&](int v) { closed_fd = v; ++close_times; });
        fd2 = std::move(fd1);
        EXPECT_EQ(close_times, 1);
        EXPECT_EQ(closed_fd, 13);
        EXPECT_TRUE(fd1.isNull());
        EXPECT_TRUE(fd2.isNull());
    }
    EXPECT_EQ(close_times, 1);
}

}
}
