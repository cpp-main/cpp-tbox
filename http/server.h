#ifndef TBOX_HTTP_SERVER_H_20220501
#define TBOX_HTTP_SERVER_H_20220501

#include "common.h"
#include <tbox/event/loop.h>
#include <tbox/network/tcp_server.h>

namespace tbox {
namespace http {

class Server {
    using namespace event;
    using namespace network;

  public:
    explicit Server(event::Loop *wp_loop);
    virtual ~Server();

    using NextFunc = std::function<void()>;
    using RequestCallback = std::function<void(RequestSptr, RespondSptr, const NextFunc &)>;

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

  public:
    void use(const RequestCallback &cb);
    void use(Middleware *wp_middleware);
};

}
}

#endif //TBOX_HTTP_SERVER_H_20220501
