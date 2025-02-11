/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <iostream>
#include <chrono>
#include <gtest/gtest.h>

#include "async_stdout_sink.h"

using namespace std;
using namespace tbox::log;

TEST(AsyncStdoutSink, DefaultLevel)
{
    AsyncStdoutSink ch;
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

    ch.cleanup();
}

TEST(AsyncStdoutSink, TraceLevel)
{
    AsyncStdoutSink ch;
    ch.enable();
    ch.setLevel("test.log", LOG_LEVEL_TRACE);
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

    ch.cleanup();
}

TEST(AsyncStdoutSink, AllLevel)
{
    AsyncStdoutSink ch;

    ch.enable();
    ch.enableColor(true);
    ch.setLevel("", LOG_LEVEL_TRACE);

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogImportant("important");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();

    ch.cleanup();
}

TEST(AsyncStdoutSink, NullString)
{
    AsyncStdoutSink ch;
    ch.enable();

    LogInfo(nullptr);
    LogPuts(LOG_LEVEL_INFO, nullptr);

    ch.cleanup();
}

TEST(AsyncStdoutSink, WillNotPrint)
{
    AsyncStdoutSink ch;
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

    ch.cleanup();
}

TEST(AsyncStdoutSink, EnableColor)
{
    AsyncStdoutSink ch;
    ch.enable();
    ch.enableColor(true);
    ch.setLevel("test.log", LOG_LEVEL_TRACE);
    cout << "Should with color" << endl;

    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();

    ch.cleanup();
}


TEST(AsyncStdoutSink, Format)
{
    AsyncStdoutSink ch;
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(AsyncStdoutSink, Truncate)
{
    auto origin_len = LogSetMaxLength(100);

    AsyncStdoutSink ch;
    ch.enable();
    std::string tmp(200, 'x');
    LogInfo("%s", tmp.c_str());

    ch.cleanup();
    LogSetMaxLength(origin_len);
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(AsyncStdoutSink, Benchmark)
{
    AsyncStdoutSink ch;
    ch.enable();
    std::string tmp(30, 'x');

    auto sp_loop = Loop::New();

    int counter = 0;
    function<void()> func = [&] {
        for (int i = 0; i < 100; ++i)
            LogInfo("%d %s", i, tmp.c_str());
        sp_loop->runNext(func);
        counter += 100;
    };
    sp_loop->runNext(func);

    sp_loop->exitLoop(chrono::seconds(10));
    sp_loop->runLoop();

    delete sp_loop;
    ch.cleanup();

    cout << "count in sec: " << counter/10 << endl;
}

