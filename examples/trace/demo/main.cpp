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
#include <tbox/trace/recorder.h>
#include <tbox/trace/sink.h>

using namespace tbox;

void Bar()
{
    RECORD_SCOPE();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
void Foo()
{
    RECORD_SCOPE();
    Bar();
    RECORD_START(abc);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    RECORD_STOP(abc);
}

void Do() {
    for (int i = 0; i < 100; ++i)
        Foo();
}

int main(int argc, char **argv)
{
    auto &ts = trace::Sink::GetInstance();
    ts.setRecordFileMaxSize(1<<20);
    ts.setPathPrefix("/tmp/test/trace-demo");
    ts.enable();

    RECORD_EVENT();

    auto t = std::thread(Do);
    Do();

    t.join();

    RECORD_EVENT();
    ts.disable();
    return 0;
}
