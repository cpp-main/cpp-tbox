/**
 * 实现一个简单HTTP服务，不管收到什么都回复200 OK
 */

#include <iostream>

#include <tbox/network/tcp_ssl_acceptor.h>
#include <tbox/network/tcp_ssl_connection.h>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/fd_event.h>
#include <tbox/event/signal_event.h>

#include <set>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

void PrintUsage(const char *prog)
{
    cout << "Usage: " << prog << " <ip:port|localpath> <cert_file> <privkey_file>" << endl
         << "Exp  : " << prog << " 127.0.0.1:12345 server.pem server.pem" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Initialize(argv[0]);

    SockAddr bind_addr = SockAddr::FromString(argv[1]);

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    set<TcpSslConnection*> conns;

    TcpSslAcceptor acceptor(sp_loop);

    acceptor.initialize(bind_addr, 1);
    acceptor.useCertificateFile(argv[2]);
    acceptor.usePrivateKeyFile(argv[3]);
    acceptor.checkPrivateKey();

    //! 指定有Client连接上了该做的事务
    acceptor.setNewConnectionCallback(
        [&] (TcpSslConnection *new_conn) {
            //! (1) 指定Client将来断开时要做的事务
            new_conn->setDisconnectedCallback(
                [&conns, new_conn, sp_loop] {
                    conns.erase(new_conn);  //! 将自己从 conns 中删除
                    sp_loop->runNext([new_conn] { delete new_conn; });  //! 延后销毁自己
                    LogTag();
                }
            );
            new_conn->setReceiveCallback(
                [&conns, new_conn, sp_loop](Buffer &buff) {
                    LogTag();
                    buff.hasReadAll();
                    const std::string cont("HTTP/1.1 200 OK\r\nConnection: Close\r\n\r\n");
                    new_conn->send(cont.c_str(), cont.size());
                    return;
                    new_conn->disconnect();
                    conns.erase(new_conn);  //! 将自己从 conns 中删除
                    sp_loop->runNext([new_conn] { delete new_conn; });  //! 延后销毁自己
                },
                0
            );
            conns.insert(new_conn);     //! (3) 将自己注册到 conns 中
        }
    );
    acceptor.start();

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [sp_loop, &conns] (int) {
            for (auto conn : conns) {
                conn->disconnect(); //! (1) 主动断开连接
                delete conn;        //! (2) 销毁Client对象。思考：为什么这里可以直接delete，而L51不可以？
            }
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    LogInfo("service runing ...");

    sp_loop->runLoop();
    LogInfo("service stoped");

    return 0;
}
