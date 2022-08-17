#include <gtest/gtest.h>
#include "file_async_channel.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace tbox::log;

TEST(FileAsyncChannel, Format)
{
    FileAsyncChannel ch;

    ch.initialize("/tmp/tbox", "test");
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(FileAsyncChannel, LongString)
{
    FileAsyncChannel ch;

    ch.initialize("/tmp/tbox", "test");
    ch.enable();

    for (size_t s = 900; s < 1200; ++s) {
        std::string tmp(s, 'z');
        LogInfo("%s", tmp.c_str());
    }

    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());

    ch.cleanup();
}

TEST(FileAsyncChannel, FileDivide)
{
    FileAsyncChannel ch;

    ch.initialize("/tmp/tbox", "test");
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
TEST(FileAsyncChannel, ParamNormalize)
{
    FileAsyncChannel ch;
    ch.initialize("  /tmp/tbox ", " test ");
    ch.enable();
    std::string tmp(120, 'v');
    LogInfo("Test LogPath");
    ch.cleanup();
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(FileAsyncChannel, Benchmark)
{
    FileAsyncChannel ch;
    ch.initialize("/tmp/tbox", "test");
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
    ch.cleanup();
}

