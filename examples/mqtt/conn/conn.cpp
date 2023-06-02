#include <iostream>
#include <cstring>

#include <base/scope_exit.hpp>
#include <event/loop.h>
#include <event/signal_event.h>
#include <base/log.h>
#include <base/log_output.h>
#include <mqtt/client.h>

using namespace std;

int main(int argc, char **argv)
{
    using namespace tbox;
    using namespace tbox::event;

    LogOutput_Initialize();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });
    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;
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
