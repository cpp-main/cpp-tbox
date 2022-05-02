#ifndef TBOX_HTTP_SERVER_H_20220502
#define TBOX_HTTP_SERVER_H_20220502

#include <vector>
#include <map>
#include "server.h"
#include <tbox/network/tcp_server.h>
#include "request_parser.h"

namespace tbox {
namespace http {

using namespace event;
using namespace network;
using namespace std;

class Server::Impl {
  public:
    Impl(event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const SockAddr &bind_addr, int listen_backlog);
    bool start();
    void stop();
    void cleanup();

  public:
    void use(const RequestCallback &cb);
    void use(Middleware *wp_middleware);

    void commitRespond(const ConnToken &ct, int index, string &content);

  private:

    void onTcpConnected(const ConnToken &ct);
    void onTcpDisconnected(const ConnToken &ct);
    void onTcpReceived(const ConnToken &ct, Buffer &buff);

    struct Connection {
        int req_index = 0;
        int res_index = 0;
        map<int, string> res_buff;
    };

    void handle(ContextSptr ctx, size_t index);

  private:
    Loop *wp_loop_;
    TcpServer tcp_server_;
    RequestParser req_parser_;
    vector<RequestCallback> req_cb_;
};

}
}

#endif //endif //endif //TBOX_HTTP_SERVER_H_20220502
