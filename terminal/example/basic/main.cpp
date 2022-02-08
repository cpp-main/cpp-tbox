#include <string>

#include <signal.h>

#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>

#include <tbox/terminal/terminal.h>
#include <tbox/terminal/telnetd.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::terminal;

void BuildNodes(TerminalBuild &term, Loop *wp_loop);

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "Exp: " << argv[0] << " 0.0.0.0:12345" << std::endl;
        return 0;
    }

    LogOutput_Initialize(argv[0]);

    auto sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    Terminal term;
    Telnetd telnetd(sp_loop, &term);
    if (!telnetd.initialize(argv[1])) {
        std::cout << "Error: telnetd init fail" << std::endl;
        return 0;
    }

    //! 注册ctrl+C停止信号
    auto *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [&] {
            telnetd.stop();
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    BuildNodes(term, sp_loop);

    telnetd.start();

    LogInfo("Start");
    sp_loop->runLoop(Loop::Mode::kForever);
    LogInfo("Stoped");

    return 0;
}

void BuildNodes(TerminalBuild &term, Loop *wp_loop)
{

}
