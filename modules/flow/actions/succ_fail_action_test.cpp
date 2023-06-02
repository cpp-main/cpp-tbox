#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(SuccAction, base) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SuccAction action(*loop);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(FailAction, base) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    FailAction action(*loop);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

}
}
