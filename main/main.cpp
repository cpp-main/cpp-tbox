#include <signal.h>
#include <vector>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/signal_event.h>

#include <tbox/util/thread_wdog.h>

#include "context.h"
#include "apps.h"
#include "args.h"

namespace tbox::main {

extern void RegisterSignals();
extern void RegisterApps(Context &context, Apps &apps); //! 由用户去实现

std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

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
    Apps apps;
    RegisterApps(context, apps);

    Json conf;
    Args args(conf);

    context.fillDefaultConfig(conf);
    apps.fillDefaultConfig(conf);

    if (!args.parse(argc, argv))
        return false;

    if (!apps.empty()) {
        if (context.initialize(conf)) {
            if (apps.initialize(conf)) {
                if (apps.start()) {  //! 启动所有应用
                    auto feeddog_timer  = context.loop()->newTimerEvent();
                    auto sig_int_event  = context.loop()->newSignalEvent();
                    auto sig_term_event = context.loop()->newSignalEvent();
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
                        context.loop()->exitLoop(std::chrono::seconds(1));
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
                    context.loop()->runLoop();
                    LogInfo("Stoped");

                    util::ThreadWDog::Unregister();
                    util::ThreadWDog::Stop();
                } else {
                    LogWarn("Apps start fail");
                }

                apps.cleanup();  //! cleanup所有应用
            } else {
                LogWarn("Apps init fail");
            }

            context.cleanup();
        } else {
            LogWarn("Context init fail");
        }
    } else {
        LogWarn("No app found");
    }

    LogInfo("Bye!");
    LogOutput_Cleanup();
    return 0;
}

__attribute__((weak))
//! 定义为弱定义，默认运行时报错误提示，避免编译错误
void RegisterApps(Context &context, Apps &apps)
{
    LogWarn("You should implement this function");
}

}

__attribute__((weak))
//! 定义为弱定义，允许用户自己定义
int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
