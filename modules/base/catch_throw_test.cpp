#include "catch_throw.h"
#include <gtest/gtest.h>

#include "log_output.h"

namespace tbox {

TEST(CatchThrow, NoThrow)
{
    LogOutput_Initialize();

    bool tag = false;
    bool has_catch = CatchThrow([&]{
        tag = true;
    });

    EXPECT_TRUE(tag);
    EXPECT_FALSE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowRuntimeError)
{
    LogOutput_Initialize();

    bool tag = false;
    bool has_catch = CatchThrow([&]{
        throw std::runtime_error("test");
        tag = true;
    });

    EXPECT_FALSE(tag);
    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowInt)
{
    LogOutput_Initialize();

    bool has_catch = CatchThrow([&]{
        throw 100;
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowRawString)
{
    LogOutput_Initialize();

    bool has_catch = CatchThrow([&]{
        throw "this is const char*";
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowStdString)
{
    LogOutput_Initialize();

    bool has_catch = CatchThrow([&]{
        throw std::string("this is std::string");
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowCustomType)
{
    LogOutput_Initialize();

    struct CustomType {};
    bool has_catch = CatchThrow([&]{
        throw CustomType();
    });

    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
}

TEST(CatchThrow, ThrowPrintStack)
{
    LogOutput_Initialize();

    bool has_catch = CatchThrow([&]{
        throw 10;
    }, true);

    EXPECT_TRUE(has_catch);

    LogOutput_Cleanup();
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
