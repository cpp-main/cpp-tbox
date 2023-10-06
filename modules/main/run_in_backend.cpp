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
struct Runtime {
    Log log;
    ContextImp ctx;
    Module apps;

    util::PidFile pid_file;
    int loop_exit_wait = 1;
    bool error_exit_wait = false;
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
    warn_signal->setCallback([](int signo) { LogWarn("Got signal %d", signo); });

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
    LogInfo("Bye!");

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

    log.fillDefaultConfig(js_conf);
    ctx.fillDefaultConfig(js_conf);
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

    util::json::GetField(js_conf, "loop_exit_wait", _runtime->loop_exit_wait);
    util::json::GetField(js_conf, "error_exit_wait", _runtime->error_exit_wait);

    log.initialize(argv[0], ctx, js_conf);
    LogOutput_Disable();

    SayHello();

    error_exit_func = [&] {
        log.cleanup();

        while (_runtime->error_exit_wait)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    if (ctx.initialize(js_conf)) {
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
    LogInfo("stop main");

    if (_runtime == nullptr) {
        std::cerr << "Err: process not start" << std::endl;
        return;
    }

    _runtime->ctx.loop()->runInLoop(
        [] {
            _runtime->apps.stop();
            _runtime->ctx.stop();
            _runtime->ctx.loop()->exitLoop(std::chrono::seconds(_runtime->loop_exit_wait));
            LogDbg("Loop will exit after %d sec", _runtime->loop_exit_wait);
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
