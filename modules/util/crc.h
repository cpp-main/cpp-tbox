#ifndef TBOX_UTIL_CRC_H_20230104
#define TBOX_UTIL_CRC_H_20230104

#include <cstdlib>
#include <cstdint>

namespace tbox {
namespace util {

uint16_t CalcCrc16(const void *data_ptr, size_t data_size);

}
}

#endif //TBOX_UTIL_CRC_H_20230104
