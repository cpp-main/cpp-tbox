#include <iostream>
#include <signal.h>
#include <tbox/event/loop.h>
#include <tbox/event/signal_item.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void SignalCallback()
{
    cout << "Got interupt signal" << endl;
}

int main()
{
    Loop* sp_loop = Loop::New();
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    SignalItem* sp_signal = sp_loop->newSignalItem();
    sp_signal->initialize(SIGINT, Item::Mode::kPersist);
    sp_signal->setCallback(SignalCallback);
    sp_signal->enable();

    cout << "Please ctrl+c" << endl;
    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_loop;
    return 0;
}
