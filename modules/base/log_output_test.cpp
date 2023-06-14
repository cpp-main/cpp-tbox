#include <gtest/gtest.h>
#include "log.h"
#include "log_output.h"

TEST(Log, Levels)
{
    LogOutput_Enable();
    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
    LogOutput_Disable();
}

TEST(Log, Format)
{
    LogOutput_Enable();
    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
    LogOutput_Disable();
}

TEST(Log, LogPuts)
{
    LogOutput_Enable();
    LogPuts(LOG_LEVEL_INFO, "should be raw: %s, %d, %f");
    LogOutput_Disable();
}

TEST(Log, error)
{
    LogOutput_Enable();
    LogErrno(1, "");
    LogErrno(1, "no value");
    LogErrno(1, "has value:%d", 123);
    LogOutput_Disable();
}
