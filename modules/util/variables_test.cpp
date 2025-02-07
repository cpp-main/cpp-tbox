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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "variables.h"
#include <tbox/base/json.hpp>
#include <gtest/gtest.h>

namespace tbox {
namespace util {

TEST(Variables, Base) {
    Variables vars;
    EXPECT_TRUE(vars.define("a", 12));

    Json js;
    EXPECT_TRUE(vars.get("a", js));
    ASSERT_TRUE(js.is_number());
    EXPECT_EQ(js.get<int>(), 12);

    EXPECT_TRUE(vars.set("a", "hello"));
    EXPECT_TRUE(vars.get("a", js));
    ASSERT_TRUE(js.is_string());
    EXPECT_EQ(js.get<std::string>(), "hello");

    EXPECT_TRUE(vars.undefine("a"));
    EXPECT_FALSE(vars.has("a"));
}

TEST(Variables, GetTemplate) {
    Variables vars;

    EXPECT_TRUE(vars.define("b", true));
    EXPECT_TRUE(vars.define("i", 12));
    EXPECT_TRUE(vars.define("s", "hello"));
    EXPECT_TRUE(vars.define("d", 12.345));

    bool b = false;
    int  i = 0;
    std::string s;
    double d = 0;

    EXPECT_TRUE(vars.get("b", b));
    EXPECT_TRUE(vars.get("i", i));
    EXPECT_TRUE(vars.get("s", s));
    EXPECT_TRUE(vars.get("d", d));
    EXPECT_FALSE(vars.get("s", d));

    EXPECT_TRUE(b);
    EXPECT_EQ(i, 12);
    EXPECT_EQ(s, "hello");
    EXPECT_DOUBLE_EQ(d, 12.345);
}

TEST(Variables, RepeatDefine) {
    Variables vars;
    EXPECT_TRUE(vars.define("a", 12));
    EXPECT_FALSE(vars.define("a", 0));
}

TEST(Variables, UndefineNotExistVar) {
    Variables vars;
    EXPECT_FALSE(vars.undefine("a"));
}

TEST(Variables, GetNotExistVar) {
    Variables vars;
    Json js;
    EXPECT_FALSE(vars.get("a", js));
}

TEST(Variables, SetNotExistVar) {
    Variables vars;
    EXPECT_FALSE(vars.set("a", 1));
}

TEST(Variables, Parent) {
    Variables vars_pparent, vars_parent, vars_child;

    vars_parent.setParent(&vars_pparent);
    vars_child.setParent(&vars_parent);

    vars_parent.define("a", 1);
    vars_parent.define("c", 22);

    vars_child.define("b", "hello");
    vars_child.define("c", 33);

    vars_pparent.define("p", 0.223);

    EXPECT_TRUE(vars_child.has("a"));   //! 子对象应该可以看到父对象中的变量
    EXPECT_TRUE(vars_child.has("b"));
    EXPECT_TRUE(vars_child.has("c"));
    EXPECT_TRUE(vars_child.has("p"));

    {
        Json js;
        EXPECT_TRUE(vars_child.get("a", js));
        ASSERT_TRUE(js.is_number());
        EXPECT_EQ(js.get<int>(), 1);
    }

    {
        Json js;
        EXPECT_TRUE(vars_child.get("b", js));
        ASSERT_TRUE(js.is_string());
        EXPECT_EQ(js.get<std::string>(), "hello");
    }

    {
        Json js;
        EXPECT_TRUE(vars_child.get("c", js));
        ASSERT_TRUE(js.is_number());
        EXPECT_EQ(js.get<int>(), 33);
    }

    {
        EXPECT_FALSE(vars_child.set("a", "world", true));
        EXPECT_TRUE(vars_child.set("a", "world"));
        Json js;
        EXPECT_TRUE(vars_child.get("a", js));
        ASSERT_TRUE(js.is_string());
        EXPECT_EQ(js.get<std::string>(), "world");

        js.clear();
        EXPECT_TRUE(vars_parent.get("a", js));
        ASSERT_TRUE(js.is_string());
        EXPECT_EQ(js.get<std::string>(), "world");

        js.clear();
        EXPECT_FALSE(vars_child.get("a", js, true));
    }

    {
        Json js;
        EXPECT_TRUE(vars_child.get("p", js));
        ASSERT_TRUE(js.is_number());
        EXPECT_DOUBLE_EQ(js.get<double>(), 0.223);
    }

    {
        EXPECT_TRUE(vars_child.set("p", 12.3));
        Json js;
        EXPECT_TRUE(vars_pparent.get("p", js));
        ASSERT_TRUE(js.is_number());
        EXPECT_DOUBLE_EQ(js.get<double>(), 12.3);
    }
}

}
}
