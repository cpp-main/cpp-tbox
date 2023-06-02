#ifndef TBOX_TERMINAL_TCP_RPC_H_20220306
#define TBOX_TERMINAL_TCP_RPC_H_20220306

#include <event/loop.h>

namespace tbox {
namespace terminal {

class TerminalInteract;

class TcpRpc {
  public:
    TcpRpc(event::Loop *wp_loop, TerminalInteract *wp_terminal);
    ~TcpRpc();

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_TERMINAL_TCP_RPC_H_20220306
