#include "loop_thread.h"

#include <gtest/gtest.h>

namespace tbox {
namespace eventx {
namespace {

TEST(LoopThread, runNow) {
    bool tag = false;
    {
        LoopThread lp;
        lp.loop()->runInLoop([&] { tag = true; });
    }
    EXPECT_TRUE(tag);
}

TEST(LoopThread, runLater) {
    bool tag = false;
    LoopThread lp(false);
    lp.loop()->runInLoop([&] { tag = true; });
    lp.start();
    lp.stop();
    EXPECT_TRUE(tag);
}

}
}
}
