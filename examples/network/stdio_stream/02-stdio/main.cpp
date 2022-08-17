#include <iostream>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/network/stdio_stream.h>

using namespace std;
using namespace tbox;

//! 一个终端的回显示例
int main()
{
    auto sp_loop = event::Loop::New();
    auto sp_stdio = new network::StdioStream(sp_loop);

    sp_stdio->bind(sp_stdio);
    sp_stdio->enable();

    auto sp_exit = sp_loop->newSignalEvent();
    sp_exit->initialize(SIGINT, event::Event::Mode::kOneshot);
    sp_exit->setCallback(
        [=] (int) {
            cout << "Info: Exit Loop" << endl;
            sp_loop->exitLoop();
        }
    );
    sp_exit->enable();

    cout << "Info: Start Loop" << endl;
    sp_loop->runLoop();
    cout << "Info: End Loop" << endl;

    delete sp_exit;
    delete sp_stdio;
    delete sp_loop;
    return 0;
}
