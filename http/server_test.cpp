#include <gtest/gtest.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log.h>
#include <tbox/log/stdout_channel.h>
#include "server.h"
#include "context.h"

namespace tbox {
namespace http {
namespace {

using namespace event;
using namespace network;

TEST(Server, SimpleRun)
{
    log::StdoutChannel log;
    log.enable();
    log.enableColor(true);
    log.setLevel(LOG_LEVEL_TRACE);

    auto sp_loop = Loop::New();
    SetScopeExitAction([=] { delete sp_loop; });
    Server srv(sp_loop);
    EXPECT_TRUE(srv.initialize(network::SockAddr::FromString("0.0.0.0:12345"), 1));
    EXPECT_TRUE(srv.start());

    srv.use(
        [=](ContextSptr ctx, const NextFunc &next) {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "Hello!";
        }
    );

    sp_loop->runLoop();
    srv.cleanup();
}

}
}
}
