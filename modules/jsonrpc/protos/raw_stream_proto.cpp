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
#include "raw_stream_proto.h"

#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/util/json.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace jsonrpc {

void RawStreamProto::sendJson(const Json &js)
{
    if (send_data_cb_) {
        const auto &json_text = js.dump();

        if (is_log_enabled_)
            LogTrace("%s send: %s", log_label_.c_str(), json_text.c_str());

        send_data_cb_(json_text.data(), json_text.size());
    }
}

ssize_t RawStreamProto::onRecvData(const void *data_ptr, size_t data_size)
{
    TBOX_ASSERT(data_ptr != nullptr);
    const char *str_ptr = static_cast<const char*>(data_ptr);
    size_t str_size = data_size;

    for (;;) {
        if (str_size < 2)
            break;

        auto json_len = util::json::FindEndPos(str_ptr, str_size);
        if (json_len <= 0)
            break;

        std::string json_text(str_ptr, json_len);

        if (is_log_enabled_)
            LogTrace("%s recv: %s", log_label_.c_str(), json_text.c_str());

        Json js;
        bool is_throw = tbox::CatchThrow([&] { js = Json::parse(json_text); }, "tbox::jsonrpc::RawStreamProto");
        if (is_throw) {
            LogNotice("parse json fail");
            return -1;
        }

        onRecvJson(js);

        str_ptr += json_len;
        str_size -= json_len;
    }

    return data_size - str_size;
}

}
}
