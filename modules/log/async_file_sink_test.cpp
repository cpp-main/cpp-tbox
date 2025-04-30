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
#include <gtest/gtest.h>
#include "async_file_sink.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <tbox/util/fs.h>

using namespace std;
using namespace tbox;
using namespace tbox::log;

TEST(AsyncFileSink, Format)
{
    AsyncFileSink ch;

    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("test");
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(AsyncFileSink, AllLevel)
{
    AsyncFileSink ch;

    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("test");
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

TEST(AsyncFileSink, FileDivide)
{
    AsyncFileSink ch;

    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("test");
    ch.setFileMaxSize(100);
    ch.enable();

    std::string tmp(30, 'z');

    LogInfo("start");
    for (size_t s = 0; s < 20; ++s)
        LogInfo("%s", tmp.c_str());
    LogInfo("end");

    ch.cleanup();
}

//! 参数规范化
TEST(AsyncFileSink, ParamNormalize)
{
    AsyncFileSink ch;

    ch.setFilePath("  /tmp/tbox ");
    ch.setFilePrefix(" test ");
    ch.enable();
    LogInfo("Test LogPath");
    ch.cleanup();
}

TEST(AsyncFileSink, RemoveLogFileDuringWriting)
{
    AsyncFileSink ch;
    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("remove_log_file_during_writing");
    ch.enable();
    util::fs::RemoveFile(ch.currentFilePath());
    LogInfo("Hello");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(util::fs::IsFileExist(ch.currentFilePath()));
    ch.cleanup();
}

TEST(AsyncFileSink, Truncate)
{
    auto origin_len = LogSetMaxLength(100);

    AsyncFileSink ch;
    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("truncate");
    ch.enable();
    util::fs::RemoveFile(ch.currentFilePath());

    std::string tmp(200, 'x');
    LogInfo("%s", tmp.c_str());

    ch.cleanup();
    LogSetMaxLength(origin_len);
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(AsyncFileSink, Benchmark)
{
    AsyncFileSink ch;
    ch.setFilePath("/tmp/tbox");
    ch.setFilePrefix("test");
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
    cout << "count in sec: " << counter/10 << endl;
    ch.cleanup();
}

