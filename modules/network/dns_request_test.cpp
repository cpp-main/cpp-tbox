#include <gtest/gtest.h>
#include "dns_request.h"
#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/log_output.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

TEST(DnsRequest, request)
{
    LogOutput_Initialize();

    Loop *sp_loop = Loop::New();    
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    DnsRequest dns(sp_loop);
    dns.request(DomainName("www.baidu.com"),
        [](const DnsRequest::Result &result) {
            for (auto &a : result.a_vec)
                cout << "A:" << a.toString() << endl;
            for (auto &cname : result.cname_vec)
                cout << "CNAME:" << cname.get() << endl;
        }
    );
    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    LogOutput_Cleanup();
}


