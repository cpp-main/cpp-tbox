#include <iostream>

#include <tbox/event/loop.h>
#include <tbox/event/timer_item.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void TimerCallback()
{
    cout << "." << flush;
}

int main()
{
    Loop* sp_loop = Loop::New();
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    TimerItem* sp_timer = sp_loop->newTimerItem();
    sp_timer->initialize(Timespan::Millisecond(200), Item::Mode::kPersist);
    sp_timer->setCallback(TimerCallback);
    sp_timer->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_loop;
    return 0;
}
