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

const std::vector<IPAddress> dns_srv_ip_vec = {
    IPAddress::FromString("114.114.114.114"),
    IPAddress::FromString("8.8.8.8")
};


TEST(DnsRequest, request_baidu)
{
    LogOutput_Initialize();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    bool tag = false;
    DnsRequest dns(sp_loop, dns_srv_ip_vec);
    dns.request(DomainName("www.baidu.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            tag = true;
            for (auto &a : result.a_vec)
                cout << "A:" << a.toString() << endl;
            for (auto &cname : result.cname_vec)
                cout << "CNAME:" << cname.toString() << endl;
        }
    );
    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_TRUE(tag);
    LogOutput_Cleanup();
}

TEST(DnsRequest, request_not_exist_domain)
{
    LogOutput_Initialize();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    bool tag = false;
    DnsRequest dns(sp_loop, dns_srv_ip_vec);
    dns.request(DomainName("wwww.this_domain_should_not_exist.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kDomainError);
            tag = true;
        }
    );
    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_TRUE(tag);
    LogOutput_Cleanup();
}

TEST(DnsRequest, timeout)
{
    LogOutput_Initialize();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    const std::vector<IPAddress> invalid_dns_srv_ip_vec = {
        IPAddress::FromString("0.0.0.0")
    };

    bool tag = false;
    DnsRequest dns(sp_loop, invalid_dns_srv_ip_vec);
    dns.request(DomainName("www.baidu.com"),
        [&tag](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kTimeout);
            tag = true;
        }
    );
    sp_loop->exitLoop(std::chrono::seconds(6));
    sp_loop->runLoop();

    EXPECT_TRUE(tag);
    LogOutput_Cleanup();
}

