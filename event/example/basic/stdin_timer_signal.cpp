#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <tbox/event/loop.h>
#include <tbox/event/fd_item.h>
#include <tbox/event/timer_item.h>
#include <tbox/event/signal_item.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void TimerCallback()
{
    cout << "." << flush;
}

void TimerCallback2()
{
    cout << "." << endl << flush;
}

void OneshotTimerCallback()
{
    cout << "#" << flush;
}

void StdinReadCallback(short events, Loop* wp_loop, TimerItem* wp_timer)
{
    if (events & FdItem::kReadEvent) {
        char buff[32];
        size_t rsize = read(STDIN_FILENO, buff, sizeof(buff));
        buff[rsize - 1] = '\0';
        string cmd(buff);

        if (cmd == "quit") {
            wp_loop->exitLoop(Timespan::Zero());
        } else if (cmd == "start") {
            wp_timer->enable();
        } else if (cmd == "stop") {
            wp_timer->disable();
        } else if (cmd == "init") {
            wp_timer->initialize(Timespan::Millisecond(10), Item::Mode::kPersist);
        } else if (cmd == "cb2") {
            wp_timer->setCallback(TimerCallback2);
        } else if (cmd == "cb1") {
            wp_timer->setCallback(TimerCallback);
        } else {
            cout << "Unknown command" << endl;
        }
    }
}

void IntSignalCallback(Loop* wp_loop)
{
    cout << "got signal" << endl;
    wp_loop->exitLoop(Timespan::Second(3));
    cout << "exit after 3 sec" << endl;
}

int main()
{
    Loop* sp_loop = Loop::New();
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    TimerItem* sp_timer = sp_loop->newTimerItem();
    sp_timer->initialize(Timespan::Second(1), Item::Mode::kPersist);
    sp_timer->setCallback(TimerCallback);
    sp_timer->enable();

    TimerItem* sp_timer_1 = sp_loop->newTimerItem();
    sp_timer_1->initialize(Timespan::Second(5), Item::Mode::kOneshot);
    sp_timer_1->setCallback(OneshotTimerCallback);
    sp_timer_1->enable();

    FdItem* sp_stdin = sp_loop->newFdItem();
    sp_stdin->initialize(STDIN_FILENO, FdItem::kReadEvent, Item::Mode::kPersist);
    sp_stdin->setCallback(bind(StdinReadCallback, std::placeholders::_1, sp_loop, sp_timer));
    sp_stdin->enable();

    SignalItem* sp_sig_int = sp_loop->newSignalItem();
    sp_sig_int->initialize(SIGINT, Item::Mode::kPersist);
    sp_sig_int->setCallback(bind(IntSignalCallback, sp_loop));
    sp_sig_int->enable();

    cout << "commands:" << endl
         << "start -- start timer" << endl
         << "stop  -- stop timer" << endl
         << "init  -- reinitialize timer" << endl
         << "cb1   -- set callback function" << endl
         << "cb2   -- set another callback function" << endl
         << "quit  -- exit test" << endl
         << "Press Ctrl+C to exit" << endl;

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_sig_int;
    delete sp_stdin;
    delete sp_timer_1;
    delete sp_timer;
    delete sp_loop;
    return 0;
}
