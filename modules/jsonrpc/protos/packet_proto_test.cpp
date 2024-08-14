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
#include <tbox/base/defines.h>
#include <tbox/base/log_output.h>

#include "packet_proto.h"

namespace tbox {
namespace jsonrpc {

TEST(PacketProto, sendRequest) {
    LogOutput_Enable();

    PacketProto proto;
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; UNUSED_VAR(id); UNUSED_VAR(errcode); UNUSED_VAR(js_result); }
    );
    proto.setSendCallback(
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        }
    );

    proto.sendRequest(1, "test");
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(PacketProto, sendRequestWithParams) {
    Json js_send_params = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    PacketProto proto;
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, js_send_params);
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; UNUSED_VAR(id); UNUSED_VAR(errcode); UNUSED_VAR(js_result); }
    );
    proto.setSendCallback(
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        }
    );

    proto.sendRequest(1, "test", js_send_params);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(PacketProto, sendResult) {
    Json js_send_result = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    PacketProto proto;
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) { ++count; UNUSED_VAR(id); UNUSED_VAR(method); UNUSED_VAR(js_params); },
        [&] (int id, int errcode, const Json &js_result) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(js_result, js_send_result);
            ++count;
            UNUSED_VAR(errcode);
        }
    );
    proto.setSendCallback(
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        }
    );

    proto.sendResult(1, js_send_result);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(PacketProto, sendError) {
    LogOutput_Enable();

    PacketProto proto;
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) { ++count; UNUSED_VAR(id); UNUSED_VAR(method); UNUSED_VAR(js_params); },
        [&] (int id, int errcode, const Json &) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(errcode, -1000);
            ++count;
        }
    );
    proto.setSendCallback(
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        }
    );

    proto.sendError(1, -1000);
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

TEST(PacketProto, RecvUncompleteData) {
    LogOutput_Enable();

    PacketProto proto;
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        },
        nullptr
    );

    const char *str_1 = R"({"id":1,"meth)";
    const char *str_2 = R"({"id":1,"method":"test","jsonrpc":"2.0"})";
    EXPECT_EQ(proto.onRecvData(str_1, ::strlen(str_1)), -1);
    EXPECT_EQ(proto.onRecvData(str_2, ::strlen(str_2)), ::strlen(str_2));

    EXPECT_EQ(count, 1);
    LogOutput_Disable();
}

}
}
