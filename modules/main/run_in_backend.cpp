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
struct Runtime {
    Log log;
    ContextImp ctx;
    Module apps;

    util::PidFile pid_file;
    int exit_wait_sec = 1;
    bool is_fault_hup = false;
    std::thread thread;

    Runtime() : apps("", ctx) {
        RegisterApps(apps, ctx);
    }
};

Runtime* _runtime = nullptr;

void RunInBackend()
{
    auto loop = _runtime->ctx.loop();

    auto warn_signal = loop->newSignalEvent("main::RunInBackend::warn_signal");
    SetScopeExitAction([=] { delete warn_signal; });

    warn_signal->initialize({SIGPIPE, SIGHUP}, event::Event::Mode::kPersist);
    warn_signal->setCallback([](int signo) { LogNotice("Got signal %d, ingore", signo); });

    //! 启动前准备
    eventx::LoopWDog::Start();
    eventx::LoopWDog::Register(loop, "main");

    warn_signal->enable();

    LogDbg("Start!");
    loop->runLoop();
    LogDbg("Stoped");

    eventx::LoopWDog::Unregister(loop);
    eventx::LoopWDog::Stop();
}

void End()
{
    SayBye();

    LogOutput_Enable();
    _runtime->log.cleanup();

    CHECK_DELETE_RESET_OBJ(_runtime);

    UninstallErrorSignals();
}

}

bool Start(int argc, char **argv)
{
    if (_runtime != nullptr) {
        std::cerr << "Err: process started" << std::endl;
        return false;
    }

    LogOutput_Enable();

    InstallErrorSignals();
    InstallTerminate();

    _runtime = new Runtime;

    auto &log = _runtime->log;
    auto &ctx = _runtime->ctx;
    auto &apps = _runtime->apps;

    Json js_conf;
    Args args(js_conf);
    Trace trace;

    log.fillDefaultConfig(js_conf);
    ctx.fillDefaultConfig(js_conf);
    trace.fillDefaultConfig(js_conf);
    apps.fillDefaultConfig(js_conf);

    if (!args.parse(argc, argv))
        return false;

    std::string pid_filename;
    util::json::GetField(js_conf, "pid_file", pid_filename);
    if (!pid_filename.empty()) {
        if (!_runtime->pid_file.lock(pid_filename)) {
            std::cerr << "Warn: another process is running, exit" << std::endl;
            return false;
        }
    }

    util::json::GetField(js_conf, "exit_wait_sec", _runtime->exit_wait_sec);
    util::json::GetField(js_conf, "is_fault_hup", _runtime->is_fault_hup);  //! 出现错误的时候是否需要挂起

    trace.initialize(ctx, js_conf);
    log.initialize(argv[0], ctx, js_conf);
    LogOutput_Disable();

    SayHi();

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        if (_runtime->is_fault_hup)
            LogNotice("process is hup.");

        //! 主要是保存日志
        log.cleanup();

        while (_runtime->is_fault_hup)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    if (ctx.initialize(argv[0], js_conf, &apps)) {
        if (apps.initialize(js_conf)) {
            if (ctx.start()) {  //! 启动所有应用
                if (apps.start()) {
                    _runtime->thread = std::thread(RunInBackend);
                    return true;
                } else {
                    LogErr("App start fail");
                }
                ctx.stop();
            } else {
                LogErr("Ctx start fail");
            }
            apps.cleanup();
        } else {
            LogErr("Apps init fail");
        }
        ctx.cleanup();
    } else {
        LogErr("Context init fail");
    }

    End();
    return false;
}

void Stop()
{
    LogImportant("stop main");

    if (_runtime == nullptr) {
        std::cerr << "Err: process not start" << std::endl;
        return;
    }

    _runtime->ctx.loop()->runInLoop(
        [] {
            _runtime->apps.stop();
            _runtime->ctx.stop();
            _runtime->ctx.loop()->exitLoop(std::chrono::seconds(_runtime->exit_wait_sec));
            LogDbg("Loop will exit after %d sec", _runtime->exit_wait_sec);
        },
        "main::Stop"
    );
    _runtime->thread.join();

    _runtime->apps.cleanup();  //! cleanup所有应用
    _runtime->ctx.cleanup();

    End();
}

}
}
