#include <iostream>
#include <cstring>
#include <tbox/event/loop.h>
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
             << "Exp  : " << argv[0] << " #" << endl;
        return 0;
    }

    const char *topic = argv[1];

    LogOutput_Initialize(argv[0]);

    Loop* sp_loop = Loop::New();
    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;
    //! 可以添加配置项
    conf.base.client_id = "pub_test";

    int sub_mid = 0;
    mqtt::Client::Callbacks cbs;
    cbs.connected = [&] {
        mqtt.subscribe(topic, &sub_mid);
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

    LogInfo("Start");
    sp_loop->runLoop(Loop::Mode::kForever);
    LogInfo("Stoped");

    mqtt.cleanup();

    delete sp_loop;
    return 0;
}
