#include <gtest/gtest.h>
#include <tbox/base/scope_exit.hpp>

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
