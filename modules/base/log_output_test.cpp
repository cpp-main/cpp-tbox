#include <gtest/gtest.h>
#include "log.h"
#include "log_output.h"

TEST(Log, output_type)
{
    LogOutput_Initialize();
    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
    LogOutput_Cleanup();
}

TEST(Log, output_detail)
{
    LogOutput_Initialize();
    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
    LogOutput_Cleanup();
}

TEST(Log, error)
{
    LogOutput_Initialize();
    LogErrno(1, "");
    LogErrno(1, "no value");
    LogErrno(1, "has value:%d", 123);
    LogOutput_Cleanup();
}
