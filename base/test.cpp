#ifdef ENABLE_TEST

#include <gtest/gtest.h>
#include <tbox/base/log.h>

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

#endif //ENABLE_TEST
