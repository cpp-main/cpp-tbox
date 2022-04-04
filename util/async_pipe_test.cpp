#include "async_pipe.h"
#include <gtest/gtest.h>
#include <vector>

using namespace tbox::util;
using namespace std;

TEST(AsyncPipe, Nomal)
{
    vector<uint8_t> out_data;

    AsyncPipe ap;
    AsyncPipe::Config cfg;
    cfg.buff_size = 2;
    cfg.buff_num  = 1;
    cfg.interval = 10;

    cfg.cb = [&] (const void *ptr, size_t size) {
        const uint8_t *p = static_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < size; ++i) {
            out_data.push_back(p[i]);
        }
    };

    EXPECT_TRUE(ap.initialize(cfg));
    for (size_t i = 0; i < 256; ++i) {
        uint8_t v = i;
        ap.append(&v, 1);
    }
    ap.cleanup();

    EXPECT_EQ(out_data.size(), 256);
    for (size_t i = 0; i < 256; ++i) {
        EXPECT_EQ(out_data[i], i);
    }
}
