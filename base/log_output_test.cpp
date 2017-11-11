#include <gtest/gtest.h>
#include "log.h"

TEST(Log, output)
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
