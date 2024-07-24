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

    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });
    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;
    conf.auto_reconnect_enable = true;
    conf.auto_reconnect_wait_sec = 5;
#if 0
    conf.base.broker.domain = "cppmain.cpp";
    conf.base.broker.port = 1883;
    conf.base.client_id = "pub_test";
    conf.base.username = "hevake";
    conf.base.passwd = "abc123";
    conf.base.keepalive = 60;

    conf.tls.enabled = true;
    conf.tls.ca_file = "./ca.perm";
    conf.tls.ca_path = "./ca";
    conf.tls.key_file = "./key";
    conf.tls.cert_file = "./cert";
    conf.tls.is_require_peer_cert = true;
    conf.tls.is_insecure = false;
#endif

    if (!mqtt.initialize(conf, mqtt::Client::Callbacks())) {
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
