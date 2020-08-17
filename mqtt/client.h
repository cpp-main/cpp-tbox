#ifndef TBOX_MQTT_CLIENT_H_20200815
#define TBOX_MQTT_CLIENT_H_20200815

#include <tbox/event/loop.h>

namespace tbox {
namespace mqtt {

class Client {
  public:
    explicit Client(event::Loop *wp_loop);
    virtual ~Client();

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_MQTT_CLIENT_H_20200815
