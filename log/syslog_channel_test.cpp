#include <gtest/gtest.h>
#include "syslog_channel.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace tbox::log;

TEST(SyslogChannel, DefaultLevel)
{
    SyslogChannel ch;
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

TEST(SyslogChannel, TraceLevel)
{
    SyslogChannel ch;
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

TEST(SyslogChannel, WillNotPrint)
{
    SyslogChannel ch;
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

TEST(SyslogChannel, NoColor)
{
    SyslogChannel ch;
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


TEST(SyslogChannel, Format)
{
    SyslogChannel ch;
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
}

TEST(SyslogChannel, LongString)
{
    SyslogChannel ch;
    ch.enable();
    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());
}

TEST(SyslogChannel, TimeCast)
{
    SyslogChannel ch;
    ch.enable();
    std::string tmp(30, 'm');

    auto start_ts = chrono::steady_clock::now();

    for (int i = 0; i < 1000; ++i)
        LogInfo("%s", tmp.c_str());

    this_thread::sleep_for(chrono::milliseconds(10));

    for (int i = 0; i < 1000; ++i)
        LogInfo("%s", tmp.c_str());

    auto end_ts = chrono::steady_clock::now();
    chrono::microseconds time_span = chrono::duration_cast<chrono::microseconds>(end_ts - start_ts);
    cout << "timecost: " << time_span.count() << " us" << endl;
}
