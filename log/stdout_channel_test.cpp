#include <gtest/gtest.h>
#include "stdout_channel.h"
#include <iostream>
#include <chrono>
#include <thread>

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

TEST(StdoutChannel, TimeCast1)
{
    StdoutChannel ch;
    ch.enable();
    std::string tmp(30, 'm');

    auto start_ts = chrono::steady_clock::now();

    for (int i = 0; i < 10000; ++i)
        LogInfo("%s", tmp.c_str());

    auto end_ts = chrono::steady_clock::now();
    chrono::microseconds time_span = chrono::duration_cast<chrono::microseconds>(end_ts - start_ts);
    cout << "timecost: " << time_span.count() << " us" << endl;
}

TEST(StdoutChannel, TimeCast2)
{
    StdoutChannel ch;
    ch.enable();
    std::string tmp(30, 'm');

    auto start_ts = chrono::steady_clock::now();

    for (int i = 0; i < 10000; ++i) {
        LogInfo("%s", tmp.c_str());
        if (i % 100 == 0) {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }

    auto end_ts = chrono::steady_clock::now();
    chrono::microseconds time_span = chrono::duration_cast<chrono::microseconds>(end_ts - start_ts);
    cout << "timecost: " << time_span.count() << " us" << endl;
}
