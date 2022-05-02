#include <gtest/gtest.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log.h>
#include <tbox/log/stdout_channel.h>
#include "server.h"

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

    auto sp_loop = Loop::New();
    SetScopeExitAction([=] { delete sp_loop; });
    Server srv(sp_loop);
    EXPECT_TRUE(srv.initialize(network::SockAddr::FromString("0.0.0.0:12345"), 1));
    EXPECT_TRUE(srv.start());

    sp_loop->runLoop();
    srv.cleanup();
}

}
}
}
