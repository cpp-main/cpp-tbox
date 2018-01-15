/**
 * 实现一个tcp的echo服务，对方发送什么就回复什么
 */

#include <iostream>

#include <tbox/network/tcp_acceptor.h>
#include <tbox/network/tcp_connection.h>

#include <tbox/base/log.h>
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
    using namespace std;
    cout << "Usage: " << prog << " <ip:port|localpath>" << endl
         << "Exp  : " << prog << " 127.0.0.1:12345" << endl
         << "       " << prog << " /tmp/test.sock" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    set<TcpConnection*> conns;

    TcpAcceptor acceptor(sp_loop);
    acceptor.initialize(SockAddr::FromString(argv[1]), 1);
    //! 指定有Client连接上了该做的事务
    acceptor.setNewConnectionCallback(
        [&] (TcpConnection *new_conn) {
            //! (1) 指定Client将来断开时要做的事务
            new_conn->setDisconnectedCallback(
                [&conns, new_conn, sp_loop] {
                    conns.erase(new_conn);  //! 将自己从 conns 中删除
                    sp_loop->runNext([new_conn] { delete new_conn; });  //! 延后销毁自己
                }
            );
            new_conn->bind(new_conn);   //! (2) 信息流绑定为自己
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
        [sp_loop, &conns] {
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
