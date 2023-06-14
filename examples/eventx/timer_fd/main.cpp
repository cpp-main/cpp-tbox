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
