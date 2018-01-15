/**
 * 实现一个tcp的echo服务，对方发送什么就回复什么
 */

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

int main()
{
    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    set<TcpConnection*> conns;

    TcpAcceptor acceptor(sp_loop);
    acceptor.initialize(SockAddr::FromString("0.0.0.0:12345"), 1);
    acceptor.setNewConnectionCallback(
        [&] (TcpConnection *new_conn) {
            new_conn->setDisconnectedCallback(
                [&conns, new_conn, sp_loop] {
                    conns.erase(new_conn);
                    sp_loop->runNext(
                        [new_conn] {
                            LogInfo("conn:%s deleted", new_conn->peerAddr().toString().c_str());
                            delete new_conn;
                        }
                    );
                }
            );
            new_conn->bind(new_conn);
            conns.insert(new_conn);
        }
    );
    acceptor.start();

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    sp_stop_ev->setCallback([sp_loop] { sp_loop->exitLoop(); });
    sp_stop_ev->enable();

    sp_loop->runLoop();

    for (auto conn : conns) {
        LogInfo("conn:%s deleted", conn->peerAddr().toString().c_str());
        conn->disconnect();
        delete conn;
    }

    return 0;
}
