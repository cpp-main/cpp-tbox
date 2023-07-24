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
#include <iostream>
#include <cstring>

#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/mqtt/client.h>

using namespace std;

int main(int argc, char **argv)
{
    using namespace tbox;
    using namespace tbox::event;

    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <topic> <content>" << endl
             << "Exp  : " << argv[0] << " say_hello 'Hello this is a test'" << endl;
        return 0;
    }

    const char *topic = argv[1];
    const char *payload = argv[2];

    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;

    int pub_mid = 0;
    mqtt::Client::Callbacks cbs;
    cbs.connected = [&] {
        LogInfo("connected");
        mqtt.publish(topic, payload, strlen(payload), 0, false, &pub_mid);
    };
    cbs.disconnected = [] {
        LogInfo("disconnected");
    };
    cbs.message_pub = [&] (int mid) {
        if (mid == pub_mid) {
            LogInfo("publish success");
            mqtt.stop();
            sp_loop->exitLoop();
        }
    };

    if (!mqtt.initialize(conf, cbs)) {
        LogErr("init mqtt fail");
        return 0;
    }

    mqtt.start();

    LogInfo("Start");
    sp_loop->runLoop(Loop::Mode::kForever);
    LogInfo("Stoped");

    mqtt.cleanup();
    return 0;
}
