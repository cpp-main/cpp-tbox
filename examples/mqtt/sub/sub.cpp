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
#include <tbox/event/signal_event.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/mqtt/client.h>

using namespace std;

int main(int argc, char **argv)
{
    using namespace tbox;
    using namespace tbox::event;

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <topic>" << endl
             << "Exp  : " << argv[0] << " '#'" << endl;
        return 0;
    }

    const char *topic = argv[1];
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;

    int sub_mid = 0;
    mqtt::Client::Callbacks cbs;
    cbs.connected = [&] {
        LogInfo("connected");
        mqtt.subscribe(topic, &sub_mid);
    };
    cbs.disconnected = [] {
        LogInfo("disconnected");
    };
    cbs.subscribed = [&] (int mid, int, const int *) {
        if (mid == sub_mid)
            cout << "subscribe success" << endl;
    };
    cbs.message_recv = [&] (int mid, const string &topic,
                            const void *payload_ptr, int payload_len,
                            int qos, bool retain) {
        cout << topic << ' ' << static_cast<const char *>(payload_ptr) << endl;
    };

    if (!mqtt.initialize(conf, cbs)) {
        LogErr("init mqtt fail");
        return 0;
    }

    mqtt.start();

    auto stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([stop_ev] { delete stop_ev; });
    stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    stop_ev->enable();
    stop_ev->setCallback(
        [sp_loop, &mqtt] (int) {
            mqtt.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("Start");
    sp_loop->runLoop(Loop::Mode::kForever);
    LogInfo("Stoped");

    mqtt.cleanup();
    return 0;
}
