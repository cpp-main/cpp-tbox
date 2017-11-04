#include <gtest/gtest.h>
#include "log.h"
#include "scope_exit.hpp"

TEST(base_log, output)
{
    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}

TEST(base_scope_exit_action, no_name)
{
    bool tag = false;
    {
        SetScopeExitAction([&] {tag = true;});
        EXPECT_FALSE(tag);
    }
    EXPECT_TRUE(tag);
}

TEST(base_scope_exit_action, named)
{
    bool tag = false;
    {
        tbox::ScopeExitAction a1([&] {tag = true;});
        EXPECT_FALSE(tag);
    }
    EXPECT_TRUE(tag);
}

TEST(base_scope_exit_action, cancel)
{
    bool tag = false;
    {
        tbox::ScopeExitAction a1([&] {tag = true;});
        EXPECT_FALSE(tag);
        a1.cancel();
    }
    EXPECT_FALSE(tag);
}

