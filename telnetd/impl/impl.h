#include "../telnetd.h"
#include <tbox/network/tcp_server.h>

namespace tbox::telnetd {

using namespace event;
using namespace network;

class Telnetd::Impl {
  public:
    Impl(event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const std::string &bind_addr);
    bool start();
    void stop();
    void cleanup();

  protected:
    void onTcpConnected(const TcpServer::Client &client);
    void onTcpDisconnected(const TcpServer::Client &client);
    void onTcpReceived(const TcpServer::Client &client, Buffer &buff);

  private:
    Loop *wp_loop_ = nullptr;
    TcpServer *sp_tcp_ = nullptr;
};

}
