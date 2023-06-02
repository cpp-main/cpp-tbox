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
    UdpSocket recv(sp_loop);
    recv.bind(SockAddr::FromString("127.0.0.1:6666"));
    recv.setRecvCallback(
        [] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            const char *str = (const char *)data_ptr;
            cout << "recv from " << from.toString() << " : " << str << endl;
        }
    );
    recv.enable();

    cout << "start" << endl;
    sp_loop->runLoop();
    cout << "exit" << endl;

    return 0;
}
