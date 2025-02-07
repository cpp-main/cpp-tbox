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
#include "trace.h"

namespace tbox {
namespace main {

extern void InstallErrorSignals();
extern void UninstallErrorSignals();

extern void InstallTerminate();

extern void RegisterApps(Module &root, Context &ctx);

extern void SayHi();
extern void SayBye();

extern std::function<void()> error_exit_func;

namespace {
void RunInFrontend(ContextImp &ctx, Module &apps, int exit_wait_sec)
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
            LogImportant("Got signal %d, stop", signo);
            apps.stop();
            ctx.stop();
            ctx.loop()->exitLoop(std::chrono::seconds(exit_wait_sec));
            LogDbg("Loop will exit after %d sec", exit_wait_sec);
        }
    );

    warn_signal->initialize({SIGPIPE, SIGHUP}, event::Event::Mode::kPersist);
    warn_signal->setCallback([](int signo) { LogNotice("Got signal %d, ignore", signo); });

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
    Trace trace;

    log.fillDefaultConfig(js_conf);
    ctx.fillDefaultConfig(js_conf);
    trace.fillDefaultConfig(js_conf);
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

    int exit_wait_sec = 1;
    util::json::GetField(js_conf, "exit_wait_sec", exit_wait_sec);

    bool is_fault_hup = false;
    util::json::GetField(js_conf, "is_fault_hup", is_fault_hup);  //! 出现错误的时候是否需要挂起

    trace.initialize(ctx, js_conf);
    log.initialize(argv[0], ctx, js_conf);
    LogOutput_Disable();

    SayHi();

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        if (is_fault_hup)
            LogNotice("process is hup.");

        //! 主要是保存日志
        log.cleanup();

        while (is_fault_hup)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    if (ctx.initialize(argv[0], js_conf, &apps)) {
        if (apps.initialize(js_conf)) {
            if (ctx.start() && apps.start()) {  //! 启动所有应用
                RunInFrontend(ctx, apps, exit_wait_sec);
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

    SayBye();

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
