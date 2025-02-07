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
#include <tbox/main/main.h>

#include "echo_server/app.h"
#include "nc_client/app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new echo_server::App(ctx));
    apps.add(new nc_client::App(ctx));
    apps.addAs(new nc_client::App(ctx), "nc_client_2");
}

std::string GetAppDescribe()
{
    return "There are two app. One is nc_client, the other is echo_server";
}

std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
}

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 1;
    minor = 0;
    rev = 0;
    build = 1;
}

}
}
