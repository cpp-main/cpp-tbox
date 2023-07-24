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
#include "cabinet.hpp"
#include <vector>

namespace tbox {
namespace cabinet {
namespace {

TEST(Cabinet, alloc_1_and_free)
{
    Cabinet<int> oc;
    auto t = oc.alloc(new int(100));

    int *i1 = oc.at(t);
    EXPECT_EQ(oc.size(), 1u);
    EXPECT_NE(i1, nullptr);
    EXPECT_EQ(*i1, 100);

    oc.free(t);
    EXPECT_EQ(oc.size(), 0u);
    delete i1;

    int *i2 = oc.at(t);
    EXPECT_EQ(i2, nullptr);
}

TEST(Cabinet, alloc_100_and_free)
{
    using OC = Cabinet<int>;
    OC oc;

    std::vector<Token> tokens;
    //! 插入0~74的值
    for (int i = 0; i < 75; ++i) {
        auto t = oc.alloc(new int(i));
        tokens.push_back(t);
    }
    EXPECT_EQ(oc.size(), 75u);

    //! 读取前50个，应该都能读到。然后都删除元素
    for (int i = 0; i < 50; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(*p, i);

        oc.free(t);
        delete p;
    }
    EXPECT_EQ(oc.size(), 25u);

    //! 再次读取前50个，token应该都失效了
    for (int i = 0; i < 50; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_EQ(p, nullptr);
    }

    //! 插入75~99的值
    for (int i = 75; i < 100; ++i) {
        auto t = oc.alloc(new int(i));
        tokens.push_back(t);
    }
    EXPECT_EQ(oc.size(), 50u);

    //! 读取后50个，应该都能读到。然后都删除元素
    for (int i = 50; i < 100; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(*p, i);

        oc.free(t);
        delete p;
    }
    EXPECT_EQ(oc.size(), 0u);
}

TEST(Cabinet, alloc_update)
{
    cabinet::Cabinet<int> c;
    auto t = c.alloc();
    EXPECT_EQ(c.at(t), nullptr);

    int *p = new int (10);
    c.update(t, p);
    EXPECT_EQ(c.at(t), p);

    delete p;
}

TEST(Cabinet, NullToken)
{
    Cabinet<int> oc;
    auto t = oc.alloc(new int(100));
    delete oc.free(t);

    cabinet::Token null_token;
    EXPECT_EQ(oc.at(null_token), nullptr);
}

}
}
}
