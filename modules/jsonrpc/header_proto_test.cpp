#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>

#include "header_proto.h"

namespace tbox {
namespace jsonrpc {

TEST(HeaderProto, sendRequest) {
    LogOutput_Enable();

    HeaderProto proto;
    Proto::Callbacks cbs;

    int count = 0;
    cbs.recv_request_cb = \
        [&] (int id, const std::string &method, const Json &js_params) {
            EXPECT_EQ(id, 1);
            EXPECT_EQ(method, "test");
            EXPECT_EQ(js_params, Json());
            ++count;
        };
    cbs.recv_result_cb = \
        [&] (int id, const Json &js_result) { ++count; };

    cbs.recv_error_cb = \
        [&] (int id, int errcode) { ++count; };

    cbs.send_data_cb = \
        [&] (const void *data_ptr, size_t data_size) {
            proto.onRecvData(data_ptr, data_size);
        };

    proto.setCallbacks(cbs);

    proto.sendRequest(1, "test");
    EXPECT_EQ(count, 1);

    LogOutput_Disable();
}

}
}
