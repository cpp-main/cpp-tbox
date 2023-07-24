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
#include "client.h"

namespace tbox {
namespace http {
namespace client {

using namespace event;
using namespace network;

Client::Client(Loop *wp_loop)
{
    (void)wp_loop;
}

Client::~Client()
{ }

bool Client::initialize(const SockAddr &server_addr)
{
    (void)server_addr;
    return false;
}

void Client::request(const Request &req, const RespondCallback &cb)
{
    (void)req;
    (void)cb;
}

void Client::cleanup()
{ }

}
}
}
