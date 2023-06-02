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

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <topic>" << endl
             << "Exp  : " << argv[0] << " '#'" << endl;
        return 0;
    }

    const char *topic = argv[1];
    LogOutput_Initialize();

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
