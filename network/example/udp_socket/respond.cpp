#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/network/udp_socket.h>
#include <tbox/event/loop.h>
#include <time.h>

using namespace std;
using namespace tbox::network;
using namespace tbox::event;

int main()
{
    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    UdpSocket respond(sp_loop);
    respond.bind(SockAddr::FromString("0.0.0.0:6668"));
    respond.setRecvCallback(
        [&respond] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            const char *str = (const char *)data_ptr;
            if (string(str) == "time?") {
                time_t now = time(nullptr);
                respond.send(&now, sizeof(now), from);
            }
        }
    );
    respond.enable();

    cout << "start" << endl;
    sp_loop->runLoop();
    cout << "exit" << endl;

    return 0;
}
