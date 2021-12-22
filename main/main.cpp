#include <vector>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/util/thread_wdog.h>

#include "context.h"
#include "app.h"

namespace tbox {
namespace main {

extern void RegisterSignals();
extern void RegisterApps(Context &context, std::vector<App*> &apps);   //! 由用户去实现

std::function<void()> normal_stop_func; //!< 正常退出前要做的事情
std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

int Main(int argc, char **argv)
{
    LogOutput_Initialize(argv[0]);
    LogInfo("Wellcome!");

    Context context;

    std::vector<App*> apps;
    RegisterApps(context, apps);

    //! 初始化所有应用
    for (auto app : apps) {
        if (!app->initialize())
            break;  //!FIXME
    }

    //! 启动所有应用
    for (auto app : apps) {
        if (!app->start())
            break;  //!FIXME
    }

    RegisterSignals();

    auto feeddog_timer = context.loop()->newTimerEvent();

    normal_stop_func = [&] {
        for (auto app : apps)
            app->stop();

        feeddog_timer->disable();
        context.loop()->exitLoop(std::chrono::seconds(1));
    };

    error_exit_func = [&] {
        //! 主要是保存日志
        LogOutput_Cleanup();
    };

    //! 创建喂狗定时器
    SetScopeExitAction([feeddog_timer] { delete feeddog_timer; } );
    feeddog_timer->initialize(std::chrono::seconds(2), event::Event::Mode::kPersist);
    feeddog_timer->setCallback(util::ThreadWDog::FeedDog);

    //! 启动前准备
    util::ThreadWDog::Register("main", 3);
    util::ThreadWDog::Start();
    feeddog_timer->enable();

    LogInfo("Start!");
    context.loop()->runLoop();
    LogInfo("Stoped");

    util::ThreadWDog::Stop();
    util::ThreadWDog::Unregister();

    //! 倒序cleanup所有应用
    for (auto rit = apps.rbegin(); rit != apps.rend(); ++rit)
        (*rit)->cleanup();

    context.thread_pool()->cleanup();

    //! 倒序释放所有应用
    for (auto rit = apps.rbegin(); rit != apps.rend(); ++rit)
        delete *rit;

    LogInfo("Bye!");
    LogOutput_Cleanup();
    return 0;
}

}
}

int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
