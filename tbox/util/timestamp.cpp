#include "timestamp.h"
#include <sys/time.h>

namespace tbox {
namespace util {

uint32_t GetCurrentSecondsFrom1970()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    return tv.tv_sec;
}

uint64_t GetCurrentMillisecondsFrom1970()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

}
}


