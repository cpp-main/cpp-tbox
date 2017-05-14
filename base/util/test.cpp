#include <gtest/gtest.h>
#include <tbox/log.h>

TEST(log, output) {
    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}
