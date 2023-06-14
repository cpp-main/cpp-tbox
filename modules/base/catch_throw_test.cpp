#include "catch_throw.h"
#include <gtest/gtest.h>

#include "log_output.h"

namespace tbox {

TEST(CatchThrow, NoThrow)
{
    LogOutput_Enable();

    bool tag = false;
    bool has_catch = CatchThrow([&]{
        tag = true;
    });

    EXPECT_TRUE(tag);
    EXPECT_FALSE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowRuntimeError)
{
    LogOutput_Enable();

    bool tag = false;
    bool has_catch = CatchThrow([&]{
        throw std::runtime_error("test");
        tag = true;
    });

    EXPECT_FALSE(tag);
    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowInt)
{
    LogOutput_Enable();

    bool has_catch = CatchThrow([&]{
        throw 100;
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowRawString)
{
    LogOutput_Enable();

    bool has_catch = CatchThrow([&]{
        throw "this is const char*";
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowStdString)
{
    LogOutput_Enable();

    bool has_catch = CatchThrow([&]{
        throw std::string("this is std::string");
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowCustomType)
{
    LogOutput_Enable();

    struct CustomType {};
    bool has_catch = CatchThrow([&]{
        throw CustomType();
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrow, ThrowPrintStack)
{
    LogOutput_Enable();

    bool has_catch = CatchThrow([&]{
        throw 10;
    }, true);

    EXPECT_TRUE(has_catch);

    LogOutput_Disable();
}

TEST(CatchThrowQuietly, Throw)
{
    bool has_catch = CatchThrowQuietly([&]{ throw 10; });
    EXPECT_TRUE(has_catch);
}

TEST(CatchThrowQuietly, NoThrow)
{
    bool has_catch = CatchThrowQuietly([&]{ });
    EXPECT_FALSE(has_catch);
}

}
