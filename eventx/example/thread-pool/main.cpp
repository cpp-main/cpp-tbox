#include <tbox/event/loop.h>
#include <tbox/event/timer_item.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/log.h>

int main()
{
    using namespace tbox::event;
    using namespace tbox::eventx;

    Loop* sp_loop = Loop::New();
    TimerItem *sp_timer = sp_loop->newTimerItem();
    ThreadPool *sp_tp = new ThreadPool(sp_loop);
    sp_tp->initialize();
    sp_timer->initialize(Timespan::Second(1), Item::Mode::kPersist);
    sp_timer->setCallback(
        [&] {
            LogInfo("put task");
            sp_tp->execute(
                []{
                    LogInfo("in sub thread");
                },
                []{
                    LogInfo("in main thread");
                }
            );
        }
    );
    sp_timer->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    sp_timer->disable();
    sp_tp->cleanup();

    delete sp_tp;
    delete sp_timer;
    delete sp_loop;
    return 0;
}
