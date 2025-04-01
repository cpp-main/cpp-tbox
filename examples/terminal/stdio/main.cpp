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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <string>
#include <signal.h>

#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log_output.h>

#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>

#include <tbox/terminal/terminal.h>
#include <tbox/terminal/service/stdio.h>

using namespace tbox;

void BuildNodes(terminal::TerminalNodes &term, event::Loop *wp_loop);

int main()
{
    //LogOutput_Enable();
    auto sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    terminal::Terminal term(sp_loop);
    terminal::Stdio stdio(sp_loop, &term);
    stdio.initialize();

    term.setWelcomeText("Welcome to Terminal STDIO demo! \r\n");

    //! 注册ctrl+C停止信号
    auto *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize({SIGINT,SIGTERM}, event::Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [&] (int) {
            stdio.stop();
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    BuildNodes(term, sp_loop);

    stdio.start();

    sp_loop->runLoop();

    stdio.cleanup();
    return 0;
}
