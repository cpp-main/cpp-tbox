#include <gtest/gtest.h>

#include "telnetd.h"
#include "shell.h"
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::shell;

TEST(Telnetd, _)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Shell shell;
    Telnetd telnet(sp_loop, &shell);

    telnet.initialize("127.0.0.1:12345");
    telnet.start();
    sp_loop->runLoop();
    telnet.cleanup();
}
