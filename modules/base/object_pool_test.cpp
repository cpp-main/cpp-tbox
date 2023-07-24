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
#include "object_pool.hpp"

namespace tbox {
namespace {

TEST(ObjectPool, Char) {
    ObjectPool<char> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc('a' + i);
        EXPECT_EQ(*p, 'a' + i);
        pool.free(p);
    }
}

TEST(ObjectPool, AllocMany) {
    ObjectPool<int> pool(50);
    std::vector<int*> tmp;

    for (int i = 0; i < 100; ++i) {
        auto p = pool.alloc(i);
        tmp.push_back(p);
    }

    for (int *p : tmp)
        pool.free(p);
    tmp.clear();

    for (int i = 0; i < 50; ++i) {
        auto p = pool.alloc(i);
        tmp.push_back(p);
    }

    for (int *p : tmp)
        pool.free(p);
    tmp.clear();
}

TEST(ObjectPool, Int) {
    ObjectPool<int> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc(123);
        EXPECT_EQ(*p, 123);
        pool.free(p);
    }
}

TEST(ObjectPool, MyStruct) {
    struct MyStruct {
        MyStruct(int i) : i_(i) { }
        char rev[100];
        int i_;
    };

    ObjectPool<MyStruct> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc(123);
        EXPECT_EQ(p->i_, 123);
        pool.free(p);
    }
}

TEST(ObjectPool, Stat_1) {
    ObjectPool<int> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc();
        pool.free(p);
    }

    auto stat = pool.getStat();
    EXPECT_EQ(stat.total_alloc_times, 10);
    EXPECT_EQ(stat.total_free_times, 10);
    EXPECT_EQ(stat.peak_alloc_number, 1);
    EXPECT_EQ(stat.peak_free_number, 1);
}

TEST(ObjectPool, Stat_2) {
    ObjectPool<int> pool(5);
    std::vector<int*> vec;

    //! 先申请10
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc();
        vec.push_back(p);
    }

    auto stat = pool.getStat();
    EXPECT_EQ(stat.total_alloc_times, 10);
    EXPECT_EQ(stat.total_free_times, 0);
    EXPECT_EQ(stat.peak_alloc_number, 10);
    EXPECT_EQ(stat.peak_free_number, 0);

    //! 释放10
    for (auto p : vec)
        pool.free(p);
    vec.clear();

    stat = pool.getStat();
    EXPECT_EQ(stat.total_alloc_times, 10);
    EXPECT_EQ(stat.total_free_times, 10);
    EXPECT_EQ(stat.peak_alloc_number, 10);
    EXPECT_EQ(stat.peak_free_number, 5);

    //! 再申请3个
    for (int i = 0; i < 3; ++i) {
        auto p = pool.alloc();
        vec.push_back(p);
    }

    stat = pool.getStat();
    EXPECT_EQ(stat.total_alloc_times, 13);
    EXPECT_EQ(stat.total_free_times, 10);
    EXPECT_EQ(stat.peak_alloc_number, 10);
    EXPECT_EQ(stat.peak_free_number, 5);

    //! 释放3个
    for (auto p : vec)
        pool.free(p);
    vec.clear();

    stat = pool.getStat();
    EXPECT_EQ(stat.total_alloc_times, 13);
    EXPECT_EQ(stat.total_free_times, 13);
    EXPECT_EQ(stat.peak_alloc_number, 10);
    EXPECT_EQ(stat.peak_free_number, 5);
}

}
}
