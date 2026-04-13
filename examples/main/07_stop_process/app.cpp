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
#include <tbox/main/main.h>

App::App(tbox::main::Context &ctx) :
    Module("app", ctx)
{ }

bool App::onStart()
{
    LogInfo("process will exit after 5 sec");
    ctx().timer_pool()->doAfter(std::chrono::seconds(5),
        [this] {
            tbox::main::RaiseStopSignal();
        }
    );
    return true;
}
