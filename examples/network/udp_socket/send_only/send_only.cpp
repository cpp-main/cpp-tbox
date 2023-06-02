#include <iostream>
#include <network/udp_socket.h>

using namespace std;
using namespace tbox::network;

int main()
{
    UdpSocket().send("hello", 6, SockAddr::FromString("127.0.0.1:6666"));
    cout << "done" << endl;
    return 0;
}
