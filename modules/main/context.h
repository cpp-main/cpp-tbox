#ifndef TBOX_MAIN_CONTEXT_H_20211222
#define TBOX_MAIN_CONTEXT_H_20211222

#include <chrono>

#include <event/loop.h>
#include <eventx/thread_pool.h>
#include <eventx/timer_pool.h>
#include <terminal/terminal_nodes.h>

namespace tbox {
namespace main {

//! 进程上下文
class Context {
  public:
    virtual event::Loop* loop() const = 0;
    virtual eventx::ThreadPool* thread_pool() const = 0;
    virtual eventx::TimerPool* timer_pool() const = 0;
    virtual terminal::TerminalNodes* terminal() const = 0;

    virtual std::chrono::milliseconds running_time() const = 0;
    virtual std::chrono::system_clock::time_point start_time_point() const = 0;
};

}
}

#endif //TBOX_MAIN_CONTEXT_H_20211222
