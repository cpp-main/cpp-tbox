#include <signal.h>
#include <vector>
#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/signal_event.h>

#include <tbox/util/thread_wdog.h>
#include <tbox/util/pid_file.h>
#include <tbox/util/fs.h>

#include "context.h"
#include "apps.h"
#include "args.h"

namespace tbox::main {

extern void RegisterSignals();
extern void RegisterApps(Apps &apps); //! 由用户去实现

extern void Run(Context &ctx, Apps &apps);

std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

int Main(int argc, char **argv)
{
    RegisterSignals();

    Apps apps;
    RegisterApps(apps);

    Context ctx;

    Json conf;
    Args args(conf);

    ctx.fillDefaultConfig(conf);
    apps.fillDefaultConfig(conf);

    if (!args.parse(argc, argv))
        return 0;

    util::PidFile pid_file;
    auto &js_pidfile = conf["pid_file"];
    if (js_pidfile.is_string()) {
        auto pid_filename = js_pidfile.get<std::string>();
        if (!pid_filename.empty())
            if (!pid_file.lock(js_pidfile.get<std::string>())) {
                std::cerr << "Warn: another process is running, exit" << std::endl;
                return 0;
            }
    }

    LogOutput_Initialize(util::fs::Basename(argv[0]).c_str());
    LogInfo("Wellcome!");

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        //! 主要是保存日志
        LogOutput_Cleanup();
    };

    if (!apps.empty()) {
        if (apps.construct(ctx)) {
            if (ctx.initialize(conf)) {
                if (apps.initialize(conf)) {
                    if (apps.start()) {  //! 启动所有应用
                        Run(ctx, apps);
                    } else {
                        LogWarn("Apps start fail");
                    }
                    apps.cleanup();  //! cleanup所有应用
                } else {
                    LogWarn("Apps init fail");
                }
                ctx.cleanup();
            } else {
                LogWarn("Context init fail");
            }
        } else {
            LogWarn("App construct fail");
        }
    } else {
        LogWarn("No app found");
    }

    LogInfo("Bye!");
    LogOutput_Cleanup();
    return 0;
}

void Run(Context &ctx, Apps &apps)
{
    auto feeddog_timer  = ctx.loop()->newTimerEvent();
    auto sig_int_event  = ctx.loop()->newSignalEvent();
    auto sig_term_event = ctx.loop()->newSignalEvent();
    //! 预定在离开时自动释放对象，确保无内存泄漏
    SetScopeExitAction(
        [feeddog_timer, sig_int_event, sig_term_event] {
            delete sig_term_event;
            delete sig_int_event;
            delete feeddog_timer;
        }
    );

    sig_int_event->initialize(SIGINT, event::Event::Mode::kOneshot);
    sig_term_event->initialize(SIGTERM, event::Event::Mode::kOneshot);
    auto normal_stop_func = [&] {
        LogInfo("Got stop signal");
        apps.stop();
        ctx.loop()->exitLoop(std::chrono::seconds(1));
    };
    sig_int_event->setCallback(normal_stop_func);
    sig_term_event->setCallback(normal_stop_func);

    //! 创建喂狗定时器
    feeddog_timer->initialize(std::chrono::seconds(2), event::Event::Mode::kPersist);
    feeddog_timer->setCallback(util::ThreadWDog::FeedDog);

    //! 启动前准备
    util::ThreadWDog::Start();
    util::ThreadWDog::Register("main", 3);

    feeddog_timer->enable();
    sig_int_event->enable();
    sig_term_event->enable();

    LogInfo("Start!");

    try {
        ctx.loop()->runLoop();
    } catch (const std::exception &e) {
        LogErr("catch execption: %s", e.what());
    } catch (...) {
        LogErr("catch unknown execption");
    }

    LogInfo("Stoped");

    util::ThreadWDog::Unregister();
    util::ThreadWDog::Stop();
}

}

__attribute__((weak))
//! 定义为弱定义，允许用户自己定义。
//! 另一方面，避免在 make test 时与 gtest 的 main() 冲突。
int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
