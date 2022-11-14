#include <gtest/gtest.h>
#include <thread>
#include "loop_wdog.h"
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log_output.h>

namespace tbox {
namespace eventx {

using namespace event;
using namespace std::chrono;

TEST(LoopWDog, Normal)
{
  auto sp_loop = Loop::New();
  SetScopeExitAction([=] {delete sp_loop;});

  int die_cb_count = 0;
  LoopWDog::SetLoopDieCallback(
    [&](const std::string &name) { ++die_cb_count; }
  );
  LoopWDog::Start();
  sp_loop->exitLoop(std::chrono::seconds(6));
  LoopWDog::Register(sp_loop, "main_loop");
  sp_loop->runLoop();
  LoopWDog::Unregister(sp_loop);
  LoopWDog::Stop();

  EXPECT_EQ(die_cb_count, 0);
}

TEST(LoopWDog, MainLoopBlock)
{
  auto sp_loop = Loop::New();
  SetScopeExitAction([=] {delete sp_loop;});
  LoopWDog::Register(sp_loop, "main_loop");

  int die_cb_count = 0;
  LoopWDog::SetLoopDieCallback(
    [&](const std::string &name) {
      EXPECT_EQ(name, "main_loop");
      ++die_cb_count;
    }
  );

  sp_loop->runInLoop([] { std::this_thread::sleep_for(std::chrono::seconds(2)); });
  sp_loop->exitLoop(std::chrono::seconds(7));

  LoopWDog::Start();
  sp_loop->runLoop();
  LoopWDog::Stop();

  EXPECT_EQ(die_cb_count, 1);
  LoopWDog::Unregister(sp_loop);
}

TEST(LoopWDog, WorkLoopBlock)
{
  auto sp_loop = Loop::New();
  auto sp_work_loop = Loop::New();

  SetScopeExitAction([=] {
      delete sp_loop;
      delete sp_work_loop;
    }
  );

  int die_cb_count = 0;
  LoopWDog::SetLoopDieCallback(
    [&](const std::string &name) {
      EXPECT_EQ(name, "work_loop");
      ++die_cb_count;
    }
  );

  LoopWDog::Register(sp_loop, "main_loop");
  LoopWDog::Register(sp_work_loop, "work_loop");

  sp_loop->exitLoop(std::chrono::seconds(7));

  LoopWDog::Start();
  sp_loop->runLoop();
  LoopWDog::Stop();

  LoopWDog::Unregister(sp_work_loop);
  LoopWDog::Unregister(sp_loop);

  EXPECT_EQ(die_cb_count, 2);
}

}
}
