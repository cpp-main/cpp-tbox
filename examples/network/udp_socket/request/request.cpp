#include <iostream>
#include <base/scope_exit.hpp>
#include <network/udp_socket.h>
#include <event/loop.h>

using namespace std;
using namespace tbox::network;
using namespace tbox::event;

int main()
{
    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    UdpSocket request(sp_loop);
    request.setRecvCallback(
        [sp_loop] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            time_t server_time = *(time_t*)data_ptr;
            cout << "time: " << server_time << endl;
            sp_loop->exitLoop();
        }
    );
    request.enable();

    std::string text("time?");
    request.send(text.c_str(), text.size() + 1, SockAddr::FromString("127.0.0.1:6668"));

    sp_loop->exitLoop(chrono::seconds(1));
    sp_loop->runLoop();
    return 0;
}
