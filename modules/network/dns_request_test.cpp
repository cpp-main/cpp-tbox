/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
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
    LogOutput_Enable();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    bool tag = false;
    DnsRequest dns(sp_loop, dns_srv_ip_vec);
    dns.request(DomainName("www.baidu.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            EXPECT_NE(result.a_vec.size(), 0);
            tag = true;
#if 0
            cout << "== www.baidu.com ==" << endl;
            for (auto &a : result.a_vec)
                cout << "A:" << a.ip.toString() << ", TTL:" << a.ttl << endl;
            for (auto &cname : result.cname_vec)
                cout << "CNAME:" << cname.cname.toString() << ", TTL:" << cname.ttl << endl;
#endif
        }
    );
    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_TRUE(tag);
    LogOutput_Disable();
    sp_loop->cleanup();
}

TEST(DnsRequest, request_baidu_and_bing)
{
    LogOutput_Enable();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    bool baidu_tag = false;
    DnsRequest dns(sp_loop, dns_srv_ip_vec);
    dns.request(DomainName("www.baidu.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            EXPECT_NE(result.a_vec.size(), 0);
            baidu_tag = true;
#if 0
            cout << "== www.baidu.com ==" << endl;
            for (auto &a : result.a_vec)
                cout << "A:" << a.ip.toString() << ", TTL:" << a.ttl << endl;
            for (auto &cname : result.cname_vec)
                cout << "CNAME:" << cname.cname.toString() << ", TTL:" << cname.ttl << endl;
#endif
        }
    );
    bool bing_tag = false;
    dns.request(DomainName("cn.bing.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            EXPECT_NE(result.a_vec.size(), 0);
            bing_tag = true;
#if 0
            cout << "== cn.bing.com ==" << endl;
            for (auto &a : result.a_vec)
                cout << "A:" << a.ip.toString() << ", TTL:" << a.ttl << endl;
            for (auto &cname : result.cname_vec)
                cout << "CNAME:" << cname.cname.toString() << ", TTL:" << cname.ttl << endl;
#endif
        }
    );

    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_TRUE(baidu_tag);
    EXPECT_TRUE(bing_tag);
    LogOutput_Disable();
    sp_loop->cleanup();
}

TEST(DnsRequest, request_baidu_and_bing_cancel_baidu)
{
    LogOutput_Enable();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    bool baidu_tag = false;
    DnsRequest dns(sp_loop, dns_srv_ip_vec);
    auto baidu_req_id = dns.request(DomainName("www.baidu.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            baidu_tag = true;
        }
    );
    bool bing_tag = false;
    dns.request(DomainName("cn.bing.com"),
        [&](const DnsRequest::Result &result) {
            EXPECT_EQ(result.status, DnsRequest::Result::Status::kSuccess);
            bing_tag = true;
        }
    );

    dns.cancel(baidu_req_id);

    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_FALSE(baidu_tag);
    EXPECT_TRUE(bing_tag);
    LogOutput_Disable();
    sp_loop->cleanup();
}


TEST(DnsRequest, request_not_exist_domain)
{
    LogOutput_Enable();

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
    LogOutput_Disable();
    sp_loop->cleanup();
}

TEST(DnsRequest, timeout)
{
    LogOutput_Enable();

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
    LogOutput_Disable();
    sp_loop->cleanup();
}

