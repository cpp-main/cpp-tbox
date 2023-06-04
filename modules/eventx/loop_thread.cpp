#include "loop_thread.h"
#include "loop_wdog.h"

namespace tbox {
namespace eventx {

LoopThread::LoopThread(bool run_now, const std::string &loop_name)
    : name_(loop_name)
    , loop_(event::Loop::New())
{
    if (run_now)
        start();
}

LoopThread::~LoopThread()
{
    stop();
    CHECK_DELETE_RESET_OBJ(loop_);
}

void LoopThread::start()
{
    if (is_running_)
        return;
    is_running_ = true;

    thread_ = std::thread(
        [this] {
            LoopWDog::Register(loop_, name_);
            loop_->runLoop();
            LoopWDog::Unregister(loop_);
        }
    );
}

void LoopThread::stop()
{
    if (!is_running_)
        return;

    loop_->runInLoop([this] { loop_->exitLoop(); });
    thread_.join();

    is_running_ = false;
}

}
}
