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
#include <tbox/base/json.hpp>

namespace app2 {

App::App(tbox::main::Context &ctx) :
    Module("app2", ctx)
{
    LogTag();
}

App::~App()
{
    LogTag();
}

void App::onFillDefaultConfig(tbox::Json &cfg)
{
    cfg["ok"] = true;
}

bool App::onInit(const tbox::Json &cfg)
{
    if (!cfg.contains("ok"))
        return false;

    bool ok = cfg["ok"].get<bool>();
    LogTrace("ok: %s", ok ? "true" : "false");
    return ok;
}

bool App::onStart()
{
    LogTag();
    return true;
}

void App::onStop()
{
    LogTag();
}

void App::onCleanup()
{
    LogTag();
}

}
