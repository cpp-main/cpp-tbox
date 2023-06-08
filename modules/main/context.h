#ifndef TBOX_MAIN_CONTEXT_H_20211222
#define TBOX_MAIN_CONTEXT_H_20211222

#include <chrono>

#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/eventx/timer_pool.h>
#include <tbox/terminal/terminal_nodes.h>
#include <tbox/system/system.h>

namespace tbox {
namespace main {

//! 进程上下文
class Context {
  public:
    virtual event::Loop* loop() const = 0;
    virtual eventx::ThreadPool* thread_pool() const = 0;
    virtual eventx::TimerPool* timer_pool() const = 0;
    virtual terminal::TerminalNodes* terminal() const = 0;
    virtual system::System* sys() const = 0;

    virtual std::chrono::milliseconds running_time() const = 0;
    virtual std::chrono::system_clock::time_point start_time_point() const = 0;
};

}
}

#endif //TBOX_MAIN_CONTEXT_H_20211222
