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

    ch.initialize("test", "/tmp/tbox");
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(FileAsyncChannel, LongString)
{
    FileAsyncChannel ch;

    ch.initialize("test", "/tmp/tbox");
    ch.enable();

    for (size_t s = 900; s < 1200; ++s) {
        std::string tmp(s, 'z');
        LogInfo("%s", tmp.c_str());
    }

    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());

    ch.cleanup();
}

TEST(FileAsyncChannel, TimeCast1)
{
    FileAsyncChannel ch;
    ch.initialize("test", "/tmp/tbox");
    ch.enable();
    std::string tmp(30, 'm');

    auto start_ts = chrono::steady_clock::now();

    LogInfo("start");
    for (int i = 0; i < 10000; ++i)
        LogInfo("%s", tmp.c_str());
    LogInfo("done");

    auto end_ts = chrono::steady_clock::now();

    ch.cleanup();
    chrono::microseconds time_span = chrono::duration_cast<chrono::microseconds>(end_ts - start_ts);
    cout << "timecost: " << time_span.count() << " us" << endl;
}


TEST(FileAsyncChannel, TimeCast2)
{
    FileAsyncChannel ch;
    ch.initialize("test", "/tmp/tbox");
    ch.enable();
    std::string tmp(30, 'x');

    auto start_ts = chrono::steady_clock::now();

    LogInfo("start");
    for (int i = 0; i < 10000; ++i) {
        LogInfo("%s", tmp.c_str());
        if (i % 100 == 0) {
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
    LogInfo("done");

    auto end_ts = chrono::steady_clock::now();

    ch.cleanup();
    chrono::microseconds time_span = chrono::duration_cast<chrono::microseconds>(end_ts - start_ts);
    cout << "timecost: " << time_span.count() << " us" << endl;
}

//! 测试日志文件分隔
TEST(FileAsyncChannel, FileSeparate)
{
    FileAsyncChannel ch;
    ch.initialize("test", "/tmp/tbox");
    ch.enable();
    std::string tmp(120, 'v');

    LogInfo("start");
    for (int i = 0; i < 50000; ++i) {
        LogInfo("%s", tmp.c_str());
        if (i % 100 == 0)
            this_thread::sleep_for(chrono::milliseconds(20));
    }
    LogInfo("done");

    ch.cleanup();
}

//! 参数规范化
TEST(FileAsyncChannel, ParamNormalize)
{
    FileAsyncChannel ch;
    ch.initialize(" test ", " /tmp/tbox/   ");
    ch.enable();
    std::string tmp(120, 'v');
    LogInfo("Test LogPath");
    ch.cleanup();
}
