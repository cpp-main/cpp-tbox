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

#include <unistd.h>

#include <iostream>
#include <chrono>
#include <algorithm>

#include "async_sink.h"

using namespace std;
using namespace tbox::log;

class TestAsyncSink : public AsyncSink {
  protected:
    virtual void endline() {
        cache_.push_back('\n');
    }

    virtual void flush() override {
        auto wsize = ::write(STDOUT_FILENO, cache_.data(), cache_.size()); //! 写到终端
        (void)wsize;  //! 消除警告用
        cache_.clear();
    }
};

class EmptyTestAsyncSink : public AsyncSink {
  protected:
    virtual void endline() { }
    virtual void flush() override { cache_.clear(); }
};


TEST(AsyncSink, Format)
{
    TestAsyncSink ch;

    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(AsyncSink, AllLevel)
{
    TestAsyncSink ch;

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

TEST(AsyncSink, Truncate)
{
    auto origin_len = LogSetMaxLength(100);

    TestAsyncSink ch;
    ch.enable();

    std::string tmp(200, 'x');
    LogInfo("%s", tmp.c_str());

    LogSetMaxLength(origin_len);
    ch.cleanup();
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(AsyncSink, Benchmark)
{
    TestAsyncSink ch;
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

TEST(AsyncSink, Benchmark_Empty)
{
    EmptyTestAsyncSink ch;
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

