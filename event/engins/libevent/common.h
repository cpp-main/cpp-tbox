#ifndef TBOX_EVENT_LIBEVENT_COMMON_H_20171015
#define TBOX_EVENT_LIBEVENT_COMMON_H_20171015

#include <chrono>
#include <event2/event.h>

namespace tbox {
namespace event {

struct timeval Chrono2Timeval(const std::chrono::milliseconds &ms);

}
}

#endif //TBOX_EVENT_LIBEVENT_COMMON_H_20171015
