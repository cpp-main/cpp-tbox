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
#ifndef TBOX_MAIN_CONTEXT_H_20211222
#define TBOX_MAIN_CONTEXT_H_20211222

#include <chrono>

#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/eventx/timer_pool.h>
#include <tbox/eventx/async.h>
#include <tbox/terminal/terminal_nodes.h>
#include <tbox/coroutine/scheduler.h>

namespace tbox {
namespace main {

//! 进程上下文
class Context {
  public:
    virtual event::Loop* loop() const = 0;
    virtual eventx::ThreadPool* thread_pool() const = 0;
    virtual eventx::TimerPool* timer_pool() const = 0;
    virtual eventx::Async* async() const = 0;
    virtual terminal::TerminalNodes* terminal() const = 0;
    virtual coroutine::Scheduler* coroutine() const = 0;

    virtual std::chrono::milliseconds running_time() const = 0;
    virtual std::chrono::system_clock::time_point start_time_point() const = 0;
};

}
}

#endif //TBOX_MAIN_CONTEXT_H_20211222
