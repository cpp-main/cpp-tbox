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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include <iostream>
#include <tbox/base/recorder.h>
#include <tbox/trace/sink.h>

void DoSomething()
{
    RECORD_SCOPE();
    std::cout << "do something" << std::endl;
}

int main(int argc, char **argv)
{
    std::cout << "this is trace demo" << std::endl;

    auto &ts = tbox::trace::Sink::GetInstance();
    ts.setPathPrefix("/tmp/trace/01_demo"); //! 设置记录文件目录前缀
    ts.setRecordFileMaxSize(1<<20); //! 设置记录文件大小为1MB
    ts.enable();  //! 开始记录

    RECORD_EVENT();

    std::cout << "begin" << std::endl;

    DoSomething();

    std::cout << "end" << std::endl;

    RECORD_EVENT();

    ts.disable(); //! 停止记录
    return 0;
}
