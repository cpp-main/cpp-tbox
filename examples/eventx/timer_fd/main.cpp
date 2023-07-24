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
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/eventx/timer_fd.h>

using namespace std;
using namespace tbox::event;
using namespace tbox::eventx;

int main(int argc, char **argv)
{
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([=] { delete sp_loop; });

    /// 注册SIGINT信号，使得ctrl+c能正常退出程序
    auto sp_sig_event = sp_loop->newSignalEvent();
    SetScopeExitAction([=] { delete sp_sig_event; });
    sp_sig_event->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_event->enable();
    sp_sig_event->setCallback([=](int) { sp_loop->exitLoop(); });

    /// 创建定时器，并使能
    TimerFd timer_fd(sp_loop);
    timer_fd.initialize(std::chrono::seconds(10), std::chrono::seconds(1));
    timer_fd.enable();
    timer_fd.setCallback([] { LogTag(); });

    LogInfo("Start");
    sp_loop->runLoop();
    LogInfo("Stoped");

    LogOutput_Disable();
    return 0;
}
