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

#include <cassert>
#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace nc_client {

using namespace tbox::main;
using namespace tbox::network;

App::App(tbox::main::Context &ctx) :
    Module("nc_client", ctx),
    client_(new TcpClient(ctx.loop())),
    stdio_(new StdioStream(ctx.loop()))
{ }

App::~App()
{
    CHECK_DELETE_RESET_OBJ(stdio_);
    CHECK_DELETE_RESET_OBJ(client_);
}

void App::onFillDefaultConfig(Json &cfg)
{
    cfg["server"] = "127.0.0.1:12345";
}

bool App::onInit(const tbox::Json &cfg)
{
    auto js_server = cfg["server"];
    if (!js_server.is_string())
        return false;

    if (!client_->initialize(SockAddr::FromString(js_server.get<std::string>())))
        return false;

    client_->bind(stdio_);
    stdio_->bind(client_);
    return true;
}

bool App::onStart()
{
    client_->start();
    stdio_->enable();
    return true;
}

void App::onStop()
{
    client_->stop();
}

void App::onCleanup()
{
    client_->cleanup();
}

}
