/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include <thread>
#include "loop_wdog.h"
#include <tbox/base/scope_exit.hpp>
//#include <tbox/base/log_output.h>
//#include <tbox/base/log.h>

namespace tbox {
namespace eventx {

using namespace event;
using namespace std::chrono;

TEST(LoopWDog, Normal)
{
  LoopWDog::Start();

  auto sp_loop = Loop::New();
  SetScopeExitAction([=] {delete sp_loop;});
  LoopWDog::Register(sp_loop, "main_loop");

  int die_cb_count = 0;
  LoopWDog::SetLoopBlockCallback(
    [&](const std::string &name) { ++die_cb_count; (void)name; }
  );

  sp_loop->exitLoop(std::chrono::seconds(6));
  sp_loop->runLoop();

  LoopWDog::Unregister(sp_loop);
  LoopWDog::Stop();

  EXPECT_EQ(die_cb_count, 0);
}

TEST(LoopWDog, MainLoopBlock)
{
  LoopWDog::Start();

  auto sp_loop = Loop::New();
  SetScopeExitAction([=] {delete sp_loop;});
  LoopWDog::Register(sp_loop, "main_loop");

  int die_cb_count = 0;
  LoopWDog::SetLoopBlockCallback(
    [&](const std::string &name) {
      EXPECT_EQ(name, "main_loop");
      ++die_cb_count;
    }
  );

  sp_loop->runInLoop([] { std::this_thread::sleep_for(std::chrono::seconds(7)); });
  sp_loop->exitLoop(std::chrono::seconds(8));

  sp_loop->runLoop();

  LoopWDog::Unregister(sp_loop);
  LoopWDog::Stop();

  EXPECT_EQ(die_cb_count, 1);
}

TEST(LoopWDog, WorkLoopBlock)
{
  //LogOutput_Enable();
  LoopWDog::Start();

  auto sp_loop = Loop::New();
  auto sp_work_loop = Loop::New();

  SetScopeExitAction([=] {
      delete sp_loop;
      delete sp_work_loop;
    }
  );

  int die_cb_count = 0;
  LoopWDog::SetLoopBlockCallback(
    [&](const std::string &name) {
      EXPECT_EQ(name, "work_loop");
      ++die_cb_count;
    }
  );

  LoopWDog::Register(sp_loop, "main_loop");
  LoopWDog::Register(sp_work_loop, "work_loop");

  std::thread t([=] { sp_work_loop->runLoop(); });
  sp_work_loop->runInLoop(
    [] {
      //LogTrace("begin");
      std::this_thread::sleep_for(std::chrono::seconds(7));
      //LogTrace("end");
    }
  );

  sp_loop->exitLoop(std::chrono::seconds(8));
  sp_loop->runLoop();

  sp_work_loop->runInLoop([sp_work_loop] { sp_work_loop->exitLoop(); });

  LoopWDog::Unregister(sp_work_loop);
  LoopWDog::Unregister(sp_loop);

  t.join();

  EXPECT_EQ(die_cb_count, 1);
  LoopWDog::Stop();
  //LogOutput_Disable();
}

}
}
