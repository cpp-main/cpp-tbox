#include <gtest/gtest.h>
#include "async_channel.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace tbox::log;

class TestAsyncChannel : public AsyncChannel {
  protected:
    virtual void onLogBackEnd(const std::string &log_text) override {
        cout << log_text << endl;
    }
};


TEST(AsyncChannel, Format)
{
    TestAsyncChannel ch;
    ch.initialize(TestAsyncChannel::Config());
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
}

TEST(AsyncChannel, LongString)
{
    TestAsyncChannel ch;
    ch.initialize(TestAsyncChannel::Config());
    ch.enable();
    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());
}

TEST(AsyncChannel, TimeCast)
{
    TestAsyncChannel ch;
    ch.initialize(TestAsyncChannel::Config());
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
