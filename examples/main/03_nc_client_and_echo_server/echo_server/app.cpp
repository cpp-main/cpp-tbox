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

namespace echo_server {

using namespace tbox::main;
using namespace tbox::network;

App::App(Context &ctx) :
    Module("echo_server", ctx),
    server_(new TcpServer(ctx.loop()))
{ }

App::~App()
{
    CHECK_DELETE_RESET_OBJ(server_);
}

void App::onFillDefaultConfig(Json &cfg)
{
    cfg["bind"] = "127.0.0.1:12345";
}

bool App::onInit(const tbox::Json &cfg)
{
    auto js_bind = cfg["bind"];
    if (!js_bind.is_string())
        return false;

    if (!server_->initialize(SockAddr::FromString(js_bind.get<std::string>()), 2))
        return false;

    server_->setReceiveCallback(
        [this] (const TcpServer::ConnToken &client, Buffer &buff) {
            server_->send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );
    return true;
}

bool App::onStart()
{
    return server_->start();
}

void App::onStop()
{
    server_->stop();
}

void App::onCleanup()
{
    server_->cleanup();
}

}
