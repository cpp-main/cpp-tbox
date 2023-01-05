#include <gtest/gtest.h>
#include "stdout_channel.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace tbox::log;

TEST(StdoutChannel, DefaultLevel)
{
    StdoutChannel ch;
    ch.enable();
    cout << "Should print INFO level" << endl;

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}

TEST(StdoutChannel, TraceLevel)
{
    StdoutChannel ch;
    ch.enable();
    ch.setLevel(LOG_LEVEL_TRACE);
    cout << "Should print all level" << endl;

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}

TEST(StdoutChannel, WillNotPrint)
{
    StdoutChannel ch;
    cout << "Should not print" << endl;

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}

TEST(StdoutChannel, NoColor)
{
    StdoutChannel ch;
    ch.enable();
    ch.enableColor(false);
    ch.setLevel(LOG_LEVEL_TRACE);
    cout << "Should no color" << endl;

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
}


TEST(StdoutChannel, Format)
{
    StdoutChannel ch;
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
}

TEST(StdoutChannel, LongString)
{
    StdoutChannel ch;
    ch.enable();
    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(StdoutChannel, Benchmark)
{
    StdoutChannel ch;
    ch.enable();
    std::string tmp(30, 'x');

    auto sp_loop = Loop::New();

    int counter = 0;
    function<void()> func = [&] {
        for (int i = 0; i < 100; ++i)
            LogInfo("%d %s", i, tmp.c_str());
        sp_loop->runInLoop(func);
        counter += 100;
    };
    sp_loop->runInLoop(func);

    sp_loop->exitLoop(chrono::seconds(10));
    sp_loop->runLoop();

    delete sp_loop;
    cout << "count in sec: " << counter/10 << endl;
}

