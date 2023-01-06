#include <gtest/gtest.h>
#include "crc.h"

namespace tbox {
namespace util {

TEST(Crc, CalcCrc16) {
    uint8_t tmp_data[] = {0x02, 0x0A, 0x00, 0x03, 0xE6, 0x02, 0x08, 0x03};
    uint16_t crc = CalcCrc16(tmp_data, sizeof(tmp_data));
    EXPECT_EQ(crc, 0xD7DD);
}

}
}
