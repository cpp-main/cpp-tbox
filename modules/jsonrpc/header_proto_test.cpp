#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>

#include "header_proto.h"

namespace tbox {
namespace jsonrpc {

TEST(HeaderProto, sendRequest) {
    LogOutput_Enable();

    HeaderProto proto;

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; }
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

TEST(HeaderProto, sendRequestWithParams) {
    Json js_send_params = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    HeaderProto proto;

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, js_send_params);
            ++count;
        },
        [&] (int id, int errcode, const Json &js_result) { ++count; }
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

TEST(HeaderProto, sendResult) {
    Json js_send_result = {
        {"a", 123},
        {"b", {"hello", "world", "!"}},
    };
    LogOutput_Enable();

    HeaderProto proto;

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) { ++count; },
        [&] (int id, int errcode, const Json &js_result) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(js_result, js_send_result);
            ++count;
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

TEST(HeaderProto, sendError) {
    LogOutput_Enable();

    HeaderProto proto;

    int count = 0;
    proto.setRecvCallback(
        [&] (int id, const std::string &method, const Json &js_params) { ++count; },
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

TEST(HeaderProto, RecvUncompleteData) {
    LogOutput_Enable();

    HeaderProto proto;

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

    const char *str_1 = "\xCA\xFE\x00\x00\x00\x28{\"id\":1,\"meth";
    const char *str_2 = "\xCA\xFE\x00\x00\x00\x28{\"id\":1,\"method\":\"test\",\"jsonrpc\":\"2.0\"})";
    EXPECT_EQ(proto.onRecvData(str_1, 19), 0);
    EXPECT_EQ(proto.onRecvData(str_2, 46), 46);

    EXPECT_EQ(count, 1);
    LogOutput_Disable();
}

}
}
