#include "common.h"

namespace tbox {
namespace event {

struct timeval Chrono2Timeval(const std::chrono::milliseconds &ms)
{
    auto ms_count = ms.count();
    struct timeval tv = {
        .tv_sec  = static_cast<time_t>(ms_count / 1000),
        .tv_usec = static_cast<time_t>((ms_count % 1000) * 1000)
    };
    return tv;
}

}
}
