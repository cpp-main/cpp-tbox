#ifndef TBOX_UTIL_TIMESTAMP_H
#define TBOX_UTIL_TIMESTAMP_H

#include <cstdint>

namespace tbox {
namespace util {

uint32_t GetCurrentSecondsFrom1970();
uint64_t GetCurrentMillisecondsFrom1970();

}
}

#endif //TBOX_UTIL_TIMESTAMP_H
