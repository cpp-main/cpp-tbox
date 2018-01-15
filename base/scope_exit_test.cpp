#include <gtest/gtest.h>
#include "scope_exit.hpp"

TEST(ScopeExitAction, no_name)
{
    bool tag = false;
    {
        SetScopeExitAction([&] {tag = true;});
        EXPECT_FALSE(tag);
    }
    EXPECT_TRUE(tag);
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
