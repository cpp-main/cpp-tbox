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
#include <iostream>
#include <thread>

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/eventx/loop_wdog.h>
#include <tbox/util/pid_file.h>
#include <tbox/util/json.h>

#include "module.h"
#include "context_imp.h"
#include "args.h"
#include "log.h"

namespace tbox {
namespace main {

extern void InstallErrorSignals();
extern void UninstallErrorSignals();

extern void InstallTerminate();

extern void RegisterApps(Module &root, Context &ctx);
extern void SayHello();

extern std::function<void()> error_exit_func;

namespace {
void RunInFrontend(ContextImp &ctx, Module &apps, int loop_exit_wait)
{
    auto stop_signal = ctx.loop()->newSignalEvent("main::RunInFrontend::stop_signal");
    auto warn_signal = ctx.loop()->newSignalEvent("main::RunInFrontend::warn_signal");

    //! 预定在离开时自动释放对象，确保无内存泄漏
    SetScopeExitAction(
        [=] {
            delete warn_signal;
            delete stop_signal;
        }
    );

    stop_signal->initialize({SIGINT, SIGTERM, SIGQUIT, SIGTSTP, SIGPWR}, event::Event::Mode::kOneshot);
    stop_signal->setCallback(
        [&] (int signo) {
            LogInfo("Got signal %d", signo);
            apps.stop();
            ctx.stop();
            ctx.loop()->exitLoop(std::chrono::seconds(loop_exit_wait));
            LogDbg("Loop will exit after %d sec", loop_exit_wait);
        }
    );

    warn_signal->initialize({SIGPIPE, SIGHUP}, event::Event::Mode::kPersist);
    warn_signal->setCallback([](int signo) { LogWarn("Got signal %d", signo); });

    //! 启动前准备
    eventx::LoopWDog::Start();
    eventx::LoopWDog::Register(ctx.loop(), "main");

    stop_signal->enable();
    warn_signal->enable();

    LogDbg("Start!");
    ctx.loop()->runLoop();
    LogDbg("Stoped");

    eventx::LoopWDog::Unregister(ctx.loop());
    eventx::LoopWDog::Stop();
}
}

int Main(int argc, char **argv)
{
    LogOutput_Enable();

    InstallErrorSignals();
    SetScopeExitAction([] { UninstallErrorSignals(); });

    InstallTerminate();

    Log log;
    ContextImp ctx;
    Module apps("", ctx);
    RegisterApps(apps, ctx);

    Json js_conf;
    Args args(js_conf);

    log.fillDefaultConfig(js_conf);
    ctx.fillDefaultConfig(js_conf);
    apps.fillDefaultConfig(js_conf);

    if (!args.parse(argc, argv))
        return 0;

    std::string pid_filename;
    util::PidFile pid_file;
    util::json::GetField(js_conf, "pid_file", pid_filename);
    if (!pid_filename.empty()) {
        if (!pid_file.lock(pid_filename)) {
            std::cerr << "Warn: another process is running, exit" << std::endl;
            return false;
        }
    }

    int loop_exit_wait = 1;
    util::json::GetField(js_conf, "loop_exit_wait", loop_exit_wait);

    bool error_exit_wait = false;
    util::json::GetField(js_conf, "error_exit_wait", error_exit_wait);

    log.initialize(argv[0], ctx, js_conf);
    LogOutput_Disable();

    SayHello();

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        //! 主要是保存日志
        log.cleanup();

        while (error_exit_wait)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    if (ctx.initialize(js_conf)) {
        if (apps.initialize(js_conf)) {
            if (ctx.start() && apps.start()) {  //! 启动所有应用
                RunInFrontend(ctx, apps, loop_exit_wait);
            } else {
                LogErr("Apps start fail");
            }
            apps.cleanup();  //! cleanup所有应用
        } else {
            LogErr("Apps init fail");
        }
        ctx.cleanup();
    } else {
        LogErr("Context init fail");
    }

    LogInfo("Bye!");

    LogOutput_Enable();
    log.cleanup();

    return 0;
}

}
}

__attribute__((weak))
//! 定义为弱定义，允许用户自己定义。
//! 另一方面，避免在 make test 时与 gtest 的 main() 冲突。
int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
