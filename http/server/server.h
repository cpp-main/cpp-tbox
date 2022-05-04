#ifndef TBOX_HTTP_SERVER_H_20220501
#define TBOX_HTTP_SERVER_H_20220501

#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"

#include "types.h"

namespace tbox {
namespace http {
namespace server {

class Middleware;

class Server {
  public:
    explicit Server(event::Loop *wp_loop);
    virtual ~Server();

  public:
    bool initialize(const network::SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

    enum class State { kNone, kInited, kRunning };
    State state() const;

  public:
    void use(const RequestCallback &cb);
    void use(Middleware *wp_middleware);

    class Impl;

  private:
    Impl *impl_;
};

}
}
}

#endif //TBOX_HTTP_SERVER_H_20220501
