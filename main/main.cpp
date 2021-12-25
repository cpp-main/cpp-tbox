#include <signal.h>
#include <vector>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/signal_event.h>
#include <tbox/util/thread_wdog.h>

#include "context.h"
#include "app.h"

namespace tbox {
namespace main {

extern void RegisterSignals();
extern void RegisterApps(Context &context, std::vector<App*> &apps);   //! 由用户去实现

std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

bool InitializeApps(const std::vector<App*> &apps);
bool StartApps(const std::vector<App*> &apps);
void StopApps(const std::vector<App*> &apps);
void CleanupApps(const std::vector<App*> &apps);
void DeleteApps(std::vector<App*> &apps);

int Main(int argc, char **argv)
{
    LogOutput_Initialize(argv[0]);
    LogInfo("Wellcome!");

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        //! 主要是保存日志
        LogOutput_Cleanup();
    };
    RegisterSignals();

    Context context;
    if (context.initialize()) {
        std::vector<App*> apps;
        RegisterApps(context, apps);

        if (!apps.empty()) {
            if (InitializeApps(apps)) {
                if (StartApps(apps)) {  //! 启动所有应用
                    auto feeddog_timer = context.loop()->newTimerEvent();
                    auto sig_int_event  = context.loop()->newSignalEvent();
                    auto sig_term_event = context.loop()->newSignalEvent();
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
                        StopApps(apps);
                        feeddog_timer->disable();
                        context.loop()->exitLoop(std::chrono::seconds(1));
                    };
                    sig_int_event->setCallback(normal_stop_func);
                    sig_term_event->setCallback(normal_stop_func);

                    //! 创建喂狗定时器
                    feeddog_timer->initialize(std::chrono::seconds(2), event::Event::Mode::kPersist);
                    feeddog_timer->setCallback(util::ThreadWDog::FeedDog);

                    //! 启动前准备
                    util::ThreadWDog::Register("main", 3);
                    util::ThreadWDog::Start();

                    feeddog_timer->enable();
                    sig_int_event->enable();
                    sig_term_event->enable();

                    LogInfo("Start!");
                    context.loop()->runLoop();
                    LogInfo("Stoped");

                    util::ThreadWDog::Stop();
                    util::ThreadWDog::Unregister();
                } else {
                    LogWarn("Start apps fail");
                }

                CleanupApps(apps);  //! cleanup所有应用
            } else {
                LogWarn("Initialize apps fail");
            }

            DeleteApps(apps);   //! 释放所有应用
        } else {
            LogWarn("No app found");
        }

        context.cleanup();
    } else {
        LogWarn("Context init fail");
    }

    LogInfo("Bye!");
    LogOutput_Cleanup();
    return 0;
}

bool InitializeApps(const std::vector<App*> &apps)
{
    for (auto app : apps) {
        if (!app->initialize())
            return false;
    }
    return true;
}

bool StartApps(const std::vector<App*> &apps)
{
    for (auto app : apps) {
        if (!app->start())
            return false;
    }
    return true;
}

void StopApps(const std::vector<App*> &apps)
{
    for (auto rit = apps.rbegin(); rit != apps.rend(); ++rit)
        (*rit)->stop();
}

void CleanupApps(const std::vector<App*> &apps)
{
    for (auto rit = apps.rbegin(); rit != apps.rend(); ++rit)
        (*rit)->cleanup();
}

void DeleteApps(std::vector<App*> &apps)
{
    while (!apps.empty()) {
        delete apps.back();
        apps.pop_back();
    }
}

}
}

int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
