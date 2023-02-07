#ifndef TBOX_NETWORK_DNS_REQUEST_H_20230207
#define TBOX_NETWORK_DNS_REQUEST_H_20230207

#include <tbox/event/loop.h>

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

    bool initialize(const std::string &domain);
    bool start();
    void stop();
    void cleanup();

    State state() const { return state_; }

  private:
    State state_ = State::kIdle;
};

}
}

#endif //TBOX_NETWORK_DNS_REQUEST_H_20230207
