#include "dns.h"
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace network {

Dns::Dns(event::Loop *loop)
    : tick_timer_(loop->newTimerEvent())
    , dns_req_(loop)
{ }

Dns::~Dns() {
    CHECK_DELETE_RESET_OBJ(tick_timer_);
}

}
}
