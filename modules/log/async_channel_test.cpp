#include <gtest/gtest.h>
#include "async_channel.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace tbox::log;

class TestAsyncChannel : public AsyncChannel {
  protected:
    virtual void onLogBackEnd(const void *data_ptr, size_t data_size) override {
        const char *start_ptr = static_cast<const char *>(data_ptr);
        if (buffer_.size() < (data_size + 1))
            buffer_.resize(data_size + 1);

        std::transform(start_ptr, start_ptr + data_size, buffer_.begin(),
            [](char ch) { return ch == 0 ? '\n' : ch; }
        );

        buffer_[data_size] = 0;
        cout << buffer_.data();
    }

  private:
    std::vector<char> buffer_;
};

class EmptyTestAsyncChannel : public AsyncChannel {
  protected:
    virtual void onLogBackEnd(const void *, size_t) override {}
};


TEST(AsyncChannel, Format)
{
    TestAsyncChannel ch;

    ch.initialize(TestAsyncChannel::Config());
    ch.enable();

    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");

    ch.cleanup();
}

TEST(AsyncChannel, LongString)
{
    TestAsyncChannel ch;

    ch.initialize(TestAsyncChannel::Config());
    ch.enable();
    std::string tmp(4096, 'x');
    LogInfo("%s", tmp.c_str());

    ch.cleanup();
}

#include <tbox/event/loop.h>
using namespace tbox::event;

TEST(AsyncChannel, Benchmark)
{
    TestAsyncChannel ch;
    ch.initialize(TestAsyncChannel::Config());
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

TEST(AsyncChannel, Benchmark_Empty)
{
    EmptyTestAsyncChannel ch;
    ch.initialize(TestAsyncChannel::Config());
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

