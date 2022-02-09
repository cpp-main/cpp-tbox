#include <string>

#include <signal.h>

#include <iostream>
#include <sstream>

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
    std::string bind_addr = "0.0.0.0:12345";
    if (argc >= 2)
        bind_addr = argv[1];

    LogOutput_Initialize(argv[0]);

    auto sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    Terminal term;
    Telnetd telnetd(sp_loop, &term);
    if (!telnetd.initialize(bind_addr)) {
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
    Func func = \
        [](Session &s, const Args &args) -> bool {
            std::stringstream ss;
            ss << "This is func. Args:";
            for (auto a : args)
                ss << ' ' << a;
            ss << "\r\n";
            s.send(ss.str());
            return true;
        };

    auto fun1_token = term.createFuncNode(func, "This is fun1");
    auto fun2_token = term.createFuncNode(func, "This is fun2");
    auto fun_token = term.createFuncNode(func, "This is fun");

    auto dir1_token = term.createDirNode("This is dir1");
    auto dir2_token = term.createDirNode();

    auto dir1_1_token = term.createDirNode();
    auto dir1_2_token = term.createDirNode();

    term.mountNode(dir1_1_token, fun_token, "func");
    term.mountNode(dir1_2_token, fun1_token, "func1");
    term.mountNode(dir1_2_token, fun2_token, "func2");

    term.mountNode(dir1_token, dir1_1_token, "dir1_1");
    term.mountNode(dir1_token, dir1_2_token, "dir1_2");

    term.mountNode(term.rootNode(), fun1_token, "func1");
    term.mountNode(term.rootNode(), dir1_token, "dir1");
    term.mountNode(term.rootNode(), dir2_token, "dir2");

    term.mountNode(dir1_1_token, term.rootNode(), "root");  //! 循环引用
}
