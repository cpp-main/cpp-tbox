#ifndef TBOX_MAIN_CONTEXT_IMP_H_20220116
#define TBOX_MAIN_CONTEXT_IMP_H_20220116

#include "context.h"

#include <tbox/base/json_fwd.h>

namespace tbox::main {

//! 进程上下文
class ContextImp : public Context {
  public:
    ContextImp();
    ~ContextImp();

    void fillDefaultConfig(Json &cfg) const;

    bool initialize(const Json &cfg);
    void cleanup();

  public:
    event::Loop* loop() const override { return sp_loop_; }
    eventx::ThreadPool* thread_pool() const override { return sp_thread_pool_; }
    eventx::Timers* timers() const override { return sp_timers_; }

  private:
    event::Loop *sp_loop_ = nullptr;
    eventx::ThreadPool *sp_thread_pool_ = nullptr;
    eventx::Timers *sp_timers_ = nullptr;
};

}

#endif //TBOX_MAIN_CONTEXT_IMP_H_20220116
