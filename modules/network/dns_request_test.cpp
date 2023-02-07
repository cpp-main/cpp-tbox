#include <gtest/gtest.h>
#include "dns_request.h"

#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/log_output.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

TEST(DnsRequest, request)
{
    LogOutput_Initialize();

    Loop *sp_loop = Loop::New();    
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    DnsRequest dns(sp_loop);
    dns.request("www.baidu.com");
    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    LogOutput_Cleanup();
}


