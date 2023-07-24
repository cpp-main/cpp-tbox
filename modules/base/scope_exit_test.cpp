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
#include "scope_exit.hpp"

namespace tbox {
namespace {

TEST(ScopeExitAction, no_name)
{
    bool tag = false;
    {
        SetScopeExitAction([&] {tag = true;});
        EXPECT_FALSE(tag);
    }
    EXPECT_TRUE(tag);
}

TEST(ScopeExitAction, no_name_1)
{
    int count = 3;
    {
        SetScopeExitAction([&] { ++count; });
        SetScopeExitAction([&] { count *= 2; });
    }
    EXPECT_EQ(count, 7);
}

TEST(ScopeExitAction, named)
{
    bool tag = false;
    {
        tbox::ScopeExitActionGuard a1([&] {tag = true;});
        EXPECT_FALSE(tag);
    }
    EXPECT_TRUE(tag);
}

TEST(ScopeExitAction, cancel)
{
    bool tag = false;
    {
        tbox::ScopeExitActionGuard a1([&] {tag = true;});
        EXPECT_FALSE(tag);
        a1.cancel();
    }
    EXPECT_FALSE(tag);
}

}
}
