#include "safe_execute.h"
#include <gtest/gtest.h>

#include <tbox/base/log_output.h>

namespace tbox {
namespace util {

TEST(SafeExecute, NoThrow)
{
    LogOutput_Initialize();

    bool tag = false;
    bool succ = SafeExecute([&]{
        tag = true;
    });

    EXPECT_TRUE(tag);
    EXPECT_TRUE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowRuntimeError)
{
    LogOutput_Initialize();

    bool tag = false;
    bool succ = SafeExecute([&]{
        throw std::runtime_error("test");
        tag = true;
    });

    EXPECT_FALSE(tag);
    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowInt)
{
    LogOutput_Initialize();

    bool succ = SafeExecute([&]{
        throw 100;
    });

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowRawString)
{
    LogOutput_Initialize();

    bool succ = SafeExecute([&]{
        throw "this is const char*";
    });

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowStdString)
{
    LogOutput_Initialize();

    bool succ = SafeExecute([&]{
        throw std::string("this is std::string");
    });

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowCustomType)
{
    LogOutput_Initialize();

    struct CustomType {};
    bool succ = SafeExecute([&]{
        throw CustomType();
    });

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowPrintStack)
{
    LogOutput_Initialize();

    bool succ = SafeExecute([&]{
        throw 10;
    }, SAFE_EXECUTE_PRINT_STACK);

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

TEST(SafeExecute, ThrowAbort)
{
    LogOutput_Initialize();

    bool succ = SafeExecute([&]{
        throw 10;
    }, SAFE_EXECUTE_ABORT);

    EXPECT_FALSE(succ);

    LogOutput_Cleanup();
}

}
}
