#include <gtest/gtest.h>
#include "filelog_channel.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <tbox/util/fs.h>

using namespace std;
using namespace tbox;
using namespace tbox::log;

TEST(FilelogChannel, Format)
{
    FilelogChannel ch;

    ch.initialize("/tmp/tbox", "test");
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(FilelogChannel, LongString)
{
    FilelogChannel ch;

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

TEST(FilelogChannel, FileDivide)
{
    FilelogChannel ch;

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
TEST(FilelogChannel, ParamNormalize)
{
    FilelogChannel ch;
    ch.initialize("  /tmp/tbox ", " test ");
    ch.enable();
    LogInfo("Test LogPath");
    ch.cleanup();
}

TEST(FilelogChannel, CreateFileInInit)
{
    FilelogChannel ch;
    ch.initialize("/tmp/tbox", "create_file_init");
    ch.enable();
    EXPECT_TRUE(util::fs::IsFileExist(ch.currentFilename()));
    ch.cleanup();
}

TEST(FilelogChannel, RemoveLogFileDuringWriting)
{
    FilelogChannel ch;
    ch.initialize("/tmp/tbox", "remove_log_file_during_writing");
    ch.enable();
    util::fs::RemoveFile(ch.currentFilename());
    LogInfo("Hello");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(util::fs::IsFileExist(ch.currentFilename()));
    ch.cleanup();
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(FilelogChannel, Benchmark)
{
    FilelogChannel ch;
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

