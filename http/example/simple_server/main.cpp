#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/log/stdout_channel.h>
#include <tbox/event/signal_event.h>
#include <tbox/http/server/server.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;

int main(int argc, char **argv)
{
    std::string bind_addr = "0.0.0.0:12345";

    if (argc == 2) {
        bind_addr = argv[1];
    }

    log::StdoutChannel log;
    log.enable();
    log.enableColor(true);
    log.setLevel(LOG_LEVEL_TRACE);

    LogInfo("enter");

    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    SetScopeExitAction(
        [=] {
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    sp_sig_event->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_event->enable();

    Server srv(sp_loop);
    if (!srv.initialize(network::SockAddr::FromString(bind_addr), 1)) {
        LogErr("init srv fail");
        return 0;
    }

    srv.start();

    srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "Hello!";
        }
    );

    sp_sig_event->setCallback(
        [&] (int) {
            srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start");
    sp_loop->runLoop();
    LogInfo("stop");
    srv.cleanup();

    LogInfo("exit");
    return 0;
}
