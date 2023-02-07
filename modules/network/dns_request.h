#ifndef TBOX_NETWORK_DNS_REQUEST_H_20230207
#define TBOX_NETWORK_DNS_REQUEST_H_20230207

#include <tbox/event/loop.h>
#include "udp_socket.h"

namespace tbox {
namespace network {

class DnsRequest {
  public:
    enum class State {
        kIdle,
        kRunning,
        kSuccess,
        kFail,
    };

    explicit DnsRequest(event::Loop *wp_loop);
    virtual ~DnsRequest();

    bool request(const std::string &domain);
    void cancel();

    State state() const { return state_; }

  protected:
    void onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from);

  private:
    State state_ = State::kIdle;
    UdpSocket udp_;
};

}
}

#endif //TBOX_NETWORK_DNS_REQUEST_H_20230207
