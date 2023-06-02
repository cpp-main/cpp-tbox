#include <iostream>
#include <base/scope_exit.hpp>
#include <event/loop.h>
#include <event/signal_event.h>
#include <network/uart.h>

using namespace std;
using namespace tbox;

void PrintUsage(const char *proc)
{
    cout << "Usage: " << proc << " <dev1> <mode1> <dev2> <mode2>" << endl
         << "Exp  : " << proc << " /dev/ttyUSB0 '115200 8n1' /dev/ttyUSB1 '19200 8n1'" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 5) {
        PrintUsage(argv[0]);
        return 0;
    }

    cout << "dev1:" << argv[1] << ", mode1:" << argv[2] << endl;
    cout << "dev2:" << argv[3] << ", mode2:" << argv[4] << endl;

    auto sp_loop  = event::Loop::New();
    SetScopeExitAction([=] { delete sp_loop; });
    auto sp_uart_1  = new network::Uart(sp_loop);
    SetScopeExitAction([=] { delete sp_uart_1; });
    auto sp_uart_2  = new network::Uart(sp_loop);
    SetScopeExitAction([=] { delete sp_uart_2; });

    if (!sp_uart_1->initialize(argv[1], argv[3]) ||
        !sp_uart_2->initialize(argv[2], argv[4])) {
        PrintUsage(argv[0]);
        return 0;
    }

    //! 将两个串口绑定到一起
    sp_uart_1->bind(sp_uart_2);
    sp_uart_2->bind(sp_uart_1);

    sp_uart_1->enable();
    sp_uart_2->enable();

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
