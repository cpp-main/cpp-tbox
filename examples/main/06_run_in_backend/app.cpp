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
#include "app.h"
#include <tbox/base/log.h>

App::App(tbox::main::Context &ctx) :
  Module("app", ctx),
  timer_(ctx.loop()->newTimerEvent())
{
  LogTag();
}

App::~App() {
  LogTag();
  delete timer_;
}

bool App::onInit(const tbox::Json &cfg) {
  LogTag();
  timer_->initialize(std::chrono::seconds(1), tbox::event::Event::Mode::kPersist);
  timer_->setCallback(
    [] {
      LogInfo("timer tick");
    }
  );
  return true;
}

bool App::onStart() {
  LogTag();
  timer_->enable();
  return true;
}

void App::onStop() {
  LogTag();
  timer_->disable();
}

void App::onCleanup() {
  LogTag();
}
