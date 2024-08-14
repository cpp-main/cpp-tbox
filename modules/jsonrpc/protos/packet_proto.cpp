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
#include "packet_proto.h"

#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/util/json.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace jsonrpc {

void PacketProto::sendJson(const Json &js)
{
    if (send_data_cb_) {
        const auto &json_text = js.dump();

        if (is_log_enabled_)
            LogTrace("%s send: %s", log_label_.c_str(), json_text.c_str());

        send_data_cb_(json_text.data(), json_text.size());
    }
}

/**
 * 相对于RawStreamProto，它不尝试寻找JSON的结束位置，减少不必要的操作
 */
ssize_t PacketProto::onRecvData(const void *data_ptr, size_t data_size)
{
    TBOX_ASSERT(data_ptr != nullptr);

    if (data_size < 2)
        return 0;

    const char *str_ptr = static_cast<const char*>(data_ptr);
    const size_t str_len = data_size;

    std::string json_text(str_ptr, str_len);

    if (is_log_enabled_)
        LogTrace("%s recv: %s", log_label_.c_str(), json_text.c_str());

    Json js;
    bool is_throw = tbox::CatchThrow([&] { js = Json::parse(json_text); });
    if (is_throw) {
        LogNotice("parse json fail");
        return -1;
    }

    onRecvJson(js);
    return str_len;
}

}
}
