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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/defines.h>

#include "protos/raw_stream_proto.h"
#include "rpc.h"

namespace tbox {
namespace jsonrpc {

class RpcIntTest : public testing::Test {
  protected:
    event::Loop *loop;
    //! 生成两个端，让它们通信
    Rpc rpc_a, rpc_b;
    RawStreamProto proto_a, proto_b;

  public:
    RpcIntTest()
        : loop(event::Loop::New())
        , rpc_a(loop)
        , rpc_b(loop)
    {
        LogOutput_Enable();
    }

    ~RpcIntTest() {
        delete loop;
        LogOutput_Disable();
    }

    void SetUp() override {
        rpc_a.initialize(&proto_a);
        rpc_b.initialize(&proto_b);
        //proto_a.setLogEnable(true);
        //proto_b.setLogEnable(true);
        
        //! a发的b收
        proto_a.setSendCallback(
            [&](const void *data_ptr, size_t data_size) {
                proto_b.onRecvData(data_ptr, data_size);
            }
        );
        //! b发的a收
        proto_b.setSendCallback(
            [&](const void *data_ptr, size_t data_size) {
                proto_a.onRecvData(data_ptr, data_size);
            }
        );
    }

    void TearDown() override {
        rpc_a.cleanup();
        rpc_b.cleanup();
        loop->cleanup();
    }
};

TEST_F(RpcIntTest, SendRequestNormally) {
    Json js_req_params = { {"a", 12}, {"b", "test jsonrpc"} };
    Json js_rsp_result = { {"r", "aabbcc"} };

    bool is_service_invoke = false;
    rpc_b.addService("A",
        [&] (int id, const Json &js_params, Response &r) {
            EXPECT_EQ(js_params, js_req_params);
            r.error.code = 0;
            r.js_result = js_rsp_result;
            is_service_invoke = true;
            UNUSED_VAR(id);
            return true;
        }
    );

    bool is_method_cb_invoke = false;
    loop->run(
        [&] {
            rpc_a.request("A", js_req_params,
                [&] (const Response &r) {
                    EXPECT_EQ(r.error.code, 0);
                    EXPECT_EQ(r.js_result, js_rsp_result);
                    is_method_cb_invoke = true;
                }
            );
        }
    );
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_service_invoke);
    EXPECT_TRUE(is_method_cb_invoke);
}

TEST_F(RpcIntTest, SendMessageNormally) {
    Json js_req_params = { {"a", 12}, {"b", "test jsonrpc"} };

    bool is_service_invoke = false;
    rpc_b.addService("B",
        [&] (int id, const Json &js_params, Response &) {
            EXPECT_EQ(id, 0);
            EXPECT_EQ(js_params, js_req_params);
            is_service_invoke = true;
            return true;
        }
    );

    loop->run(
        [&] {
            rpc_a.notify("B", js_req_params);
        }
    );
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_service_invoke);
}

TEST_F(RpcIntTest, SendMessageNoService) {
    bool is_service_invoke = false;
    rpc_b.addService("B",
        [&] (int id, const Json &, Response &) {
            is_service_invoke = true;
            UNUSED_VAR(id);
            return true;
        }
    );

    loop->run(
        [&] {
            rpc_a.notify("A");
        }
    );
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_FALSE(is_service_invoke);
}

TEST_F(RpcIntTest, SendRequestNoMethod) {
    bool is_method_cb_invoke = false;
    loop->run(
        [&] {
            rpc_a.request("A", Json(),
                [&] (const Response &r) {
                    EXPECT_EQ(r.error.code, -32601);
                    EXPECT_EQ(r.error.message, "method not found");
                    EXPECT_EQ(r.js_result, Json());
                    is_method_cb_invoke = true;
                }
            );
        }
    );
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_method_cb_invoke);
}

TEST(RpcInt, RequestTimeout) {
    auto loop = event::Loop::New();
    SetScopeExitAction([=] { delete loop; });

    Rpc rpc(loop);
    RawStreamProto proto;
    rpc.initialize(&proto, 1);

    bool is_method_cb_invoke = false;
    loop->run(
        [&] {
            rpc.request("A", Json(),
                [&] (const Response &r) {
                    EXPECT_EQ(r.error.code, -32000);
                    EXPECT_EQ(r.error.message , "request timeout");
                    UNUSED_VAR(r.js_result);
                    is_method_cb_invoke = true;
                }
            );
        }
    );
    loop->exitLoop(std::chrono::milliseconds(1001));
    loop->runLoop();

    EXPECT_TRUE(is_method_cb_invoke);
}

}
}
