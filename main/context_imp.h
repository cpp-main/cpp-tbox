#ifndef TBOX_MAIN_CONTEXT_IMP_H_20220116
#define TBOX_MAIN_CONTEXT_IMP_H_20220116

#include "context.h"

#include <tbox/base/json_fwd.h>
#include <tbox/terminal/terminal.h>
#include <tbox/terminal/service/telnetd.h>
#include <tbox/terminal/service/tcp_rpc.h>

namespace tbox::main {

//! 进程上下文
class ContextImp : public Context {
  public:
    ContextImp();
    ~ContextImp();

    void fillDefaultConfig(Json &cfg) const;

    bool initialize(const Json &cfg);
    bool start();
    void stop();
    void cleanup();

  public:
    event::Loop* loop() const override { return sp_loop_; }
    eventx::ThreadPool* thread_pool() const override { return sp_thread_pool_; }
    eventx::TimerPool* timer_pool() const override { return sp_timer_pool_; }
    terminal::TerminalNodes* terminal() const override { return sp_terminal_; }

  protected:
    void buildTerminalNodes();

  private:
    event::Loop *sp_loop_ = nullptr;
    eventx::ThreadPool *sp_thread_pool_ = nullptr;
    eventx::TimerPool  *sp_timer_pool_ = nullptr;

    terminal::Terminal *sp_terminal_ = nullptr;
    terminal::Telnetd  *sp_telnetd_ = nullptr;
    terminal::TcpRpc   *sp_tcp_rpc_ = nullptr;
    bool telnetd_init_ok = false;
    bool tcp_rpc_init_ok = false;
};

}

#endif //TBOX_MAIN_CONTEXT_IMP_H_20220116
