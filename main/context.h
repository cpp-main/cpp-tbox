#ifndef TBOX_MAIN_CONTEXT_H_20211222
#define TBOX_MAIN_CONTEXT_H_20211222

#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>

namespace tbox {
namespace main {

//! 进程上下文
class Context {
  public:
    Context();
    ~Context();

    bool initialize();  //!TODO:加参数
    void cleanup();

    inline event::Loop* loop() const { return sp_loop_; }
    inline eventx::ThreadPool* thread_pool() const { return sp_thread_pool_; }

  private:
    event::Loop *sp_loop_ = nullptr;
    eventx::ThreadPool *sp_thread_pool_ = nullptr;
};

}
}

#endif //TBOX_MAIN_CONTEXT_H_20211222
