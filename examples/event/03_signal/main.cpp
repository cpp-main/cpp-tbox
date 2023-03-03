#include <iostream>
#include <signal.h>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void SignalCallback(int signo)
{
    cout << "Got interupt signal" << endl;
}

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    SignalEvent* sp_signal = sp_loop->newSignalEvent();
    sp_signal->initialize(SIGINT, Event::Mode::kPersist);
    sp_signal->setCallback(SignalCallback);
    sp_signal->enable();

    cout << "Please ctrl+c" << endl;
    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_signal;
    delete sp_loop;
    return 0;
}
