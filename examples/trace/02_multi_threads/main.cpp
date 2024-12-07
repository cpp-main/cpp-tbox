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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include <thread>
#include <mutex>
#include <iostream>
#include <tbox/base/recorder.h>
#include <tbox/trace/sink.h>

using namespace tbox;

std::mutex g_lock;

void Bar()
{
    std::lock_guard<std::mutex> lg(g_lock);
    RECORD_SCOPE();
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    RECORD_SCOPE();
}
void Foo()
{
    RECORD_SCOPE();
    Bar();
    RECORD_SCOPE();
    std::this_thread::sleep_for(std::chrono::microseconds(5));
}

void Do() {
    RECORD_SCOPE();
    for (int i = 0; i < 3; ++i)
        Foo();
}

int main(int argc, char **argv)
{
    std::cout << "this is trace multi-threads demo" << std::endl;

    auto &ts = tbox::trace::Sink::GetInstance();
    ts.setPathPrefix("/tmp/trace/02_multi_threads"); //! 设置记录文件目录前缀
    ts.setRecordFileMaxSize(1<<20); //! 设置记录文件大小为1MB
    ts.enable();  //! 开始记录

    RECORD_EVENT();

    RECORD_DEFINE(a);
    RECORD_DEFINE(b);
    RECORD_DEFINE(c);

    RECORD_START(c);
    RECORD_START(a);
    auto t1 = std::thread(Do);
    auto t2 = std::thread(Do);
    auto t3 = std::thread(Do);
    RECORD_STOP(a);

    RECORD_START(b);
    t1.join();
    t2.join();
    t3.join();
    RECORD_STOP(b);
    RECORD_STOP(c);

    RECORD_EVENT();

    ts.disable(); //! 停止记录
    return 0;
}
