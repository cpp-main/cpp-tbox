#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/network/stdio_stream.h>
#include <tbox/network/uart.h>

using namespace std;
using namespace tbox;

void PrintUsage(const char *proc)
{
    cout << "Usage: " << proc << " <dev> <mode>" << endl
         << "Exp  : " << proc << " /dev/ttyUSB0 '115200 8n1'" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        PrintUsage(argv[0]);
        return 0;
    }

    cout << "dev:" << argv[1] << ", mode:" << argv[2] << endl;

    auto sp_loop  = event::Loop::New();
    SetScopeExitAction([=] { delete sp_loop; });
    auto sp_stdio = new network::StdioStream(sp_loop);
    SetScopeExitAction([=] { delete sp_stdio; });
    auto sp_uart  = new network::Uart(sp_loop);
    SetScopeExitAction([=] { delete sp_uart; });

    if (!sp_uart->initialize(argv[1], argv[2])) {
        PrintUsage(argv[0]);
        return 0;
    }

    //! 将终端与串口绑定到一起
    sp_stdio->bind(sp_uart);
    sp_uart->bind(sp_stdio);

    sp_stdio->enable();
    sp_uart->enable();

    auto sp_exit = sp_loop->newSignalEvent();
    SetScopeExitAction([=] { delete sp_exit; });
    sp_exit->initialize(SIGINT, event::Event::Mode::kOneshot);
    sp_exit->enable();
    sp_exit->setCallback(
        [=] (int) {
            cout << "Info: Exit Loop" << endl;
            sp_loop->exitLoop();
        }
    );

    cout << "Info: Start Loop" << endl;
    sp_loop->runLoop();
    cout << "Info: End Loop" << endl;

    return 0;
}
