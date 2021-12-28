#ifndef TBOX_MAIN_CONTEXT_H_20211222
#define TBOX_MAIN_CONTEXT_H_20211222

#include <tbox/base/json_fwd.h>
#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>

namespace tbox::main {

//! 进程上下文
class Context {
  public:
    Context();
    ~Context();

    bool initialize(const Json &cfg);
    void cleanup();

    event::Loop* loop() const;
    eventx::ThreadPool* thread_pool() const;

  private:
    struct Data;
    Data *d_ = nullptr;
};

}

#endif //TBOX_MAIN_CONTEXT_H_20211222
