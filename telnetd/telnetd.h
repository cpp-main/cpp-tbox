#ifndef TBOX_TELNET_H_20220127
#define TBOX_TELNET_H_20220127

#include <tbox/event/loop.h>

namespace tbox::telnetd {

class Telnetd {
  public:
    explicit Telnetd(event::Loop *wp_loop);
    virtual ~Telnetd();

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

#endif //TBOX_TELNET_H_20220127
