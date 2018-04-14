#ifndef TBOX_NETWORK_TCP_SERVER_H_20180412
#define TBOX_NETWORK_TCP_SERVER_H_20180412

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace network {

class TcpServer {
  public:
    explicit TcpServer(event::Loop *wp_loop);
    virtual ~TcpServer();

    NONCOPYABLE(TcpServer);
    IMMOVABLE(TcpServer);

  public:
};

}
}

#endif //TBOX_NETWORK_TCP_SERVER_H_20180412
