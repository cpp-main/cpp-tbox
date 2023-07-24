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
#ifndef TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
#define TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226

#include <tbox/main/main.h>
#include <tbox/network/tcp_server.h>

namespace echo_server {

using namespace tbox;

class App : public main::Module
{
  public:
    App(main::Context &ctx);
    virtual ~App() override;

    virtual void onFillDefaultConfig(Json &cfg) override;
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;

  private:
    network::TcpServer *server_ = nullptr;
};

}

#endif //TBOX_MAIN_EXAMPLE_ECHO_SERVER_H_20211226
