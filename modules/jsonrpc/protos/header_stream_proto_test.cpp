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
#include <tbox/base/defines.h>

#include "header_stream_proto.h"

namespace tbox {
namespace jsonrpc {

TEST(HeaderStreamProto, sendRequest) {
    LogOutput_Enable();

    HeaderStreamProto proto(0x3e5a);
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; UNUSED_VAR(id), UNUSED_VAR(errcode), UNUSED_VAR(js_result); }
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

TEST(HeaderStreamProto, sendRequestWithParams) {
    Json js_send_params = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    HeaderStreamProto proto(0x35ae);
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, js_send_params);
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; UNUSED_VAR(id), UNUSED_VAR(errcode), UNUSED_VAR(js_result); }
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

TEST(HeaderStreamProto, sendResult) {
    Json js_send_result = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    HeaderStreamProto proto(0x35ae);
    proto.setLogEnable(true);

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) { ++count; UNUSED_VAR(id), UNUSED_VAR(method), UNUSED_VAR(js_params);},
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

TEST(HeaderStreamProto, sendError) {
    LogOutput_Enable();

    HeaderStreamProto proto(0x53ea);
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

TEST(HeaderStreamProto, RecvUncompleteData) {
    LogOutput_Enable();

    HeaderStreamProto proto(0xea53);
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

    const char *str_1 = "\xEA\x53\x00\x00\x00\x28{\"id\":1,\"meth";
    const char *str_2 = "\xEA\x53\x00\x00\x00\x28{\"id\":1,\"method\":\"test\",\"jsonrpc\":\"2.0\"})";
    EXPECT_EQ(proto.onRecvData(str_1, 19), 0);
    EXPECT_EQ(proto.onRecvData(str_2, 46), 46);

    EXPECT_EQ(count, 1);
    LogOutput_Disable();
}

}
}
