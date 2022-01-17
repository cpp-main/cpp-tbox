#include <iostream>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void TimerCallback()
{
    cout << "." << flush;
}

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " libevent|libev|epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop::Engine loop_engine;
    if (string(argv[1]) == "libevent")
        loop_engine = Loop::Engine::kLibevent;
    else if (string(argv[1]) == "libev")
        loop_engine = Loop::Engine::kLibev;
    else if (string(argv[1]) == "epoll")
        loop_engine = Loop::Engine::kEpoll;
    else {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(loop_engine);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    TimerEvent* sp_timer = sp_loop->newTimerEvent();
    sp_timer->initialize(std::chrono::milliseconds(200), Event::Mode::kPersist);
    sp_timer->setCallback(TimerCallback);
    sp_timer->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_timer;
    delete sp_loop;
    return 0;
}
