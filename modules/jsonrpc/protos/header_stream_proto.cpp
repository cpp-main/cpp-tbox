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
#include "header_stream_proto.h"

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/util/json.h>
#include <tbox/util/serializer.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace jsonrpc {

const uint16_t kHeadSize = 6;   //! HeadCode(2) + ContentLength(4)

HeaderStreamProto::HeaderStreamProto(uint16_t head_code)
  : header_code_(head_code)
{ }

void HeaderStreamProto::sendJson(const Json &js)
{
    const auto &json_text = js.dump();

    if (is_log_enabled_)
        LogTrace("%s send: %s", log_label_.c_str(), json_text.c_str());

    std::vector<uint8_t> buff;
    util::Serializer pack(buff);

    pack << header_code_ << static_cast<uint32_t>(json_text.size());
    pack.append(json_text.data(), json_text.size());

    if (send_data_cb_)
        send_data_cb_(buff.data(), buff.size());
}

ssize_t HeaderStreamProto::onRecvData(const void *data_ptr, size_t data_size)
{
    TBOX_ASSERT(data_ptr != nullptr);

    if (data_size < kHeadSize)
        return 0;

    util::Deserializer unpack(data_ptr, data_size);

    uint16_t header_magic = 0;
    uint32_t content_size = 0;
    unpack >> header_magic >> content_size; 

    if (header_magic != header_code_) {
        LogNotice("head code mismatch");
        return -2;
    }

    if (content_size + kHeadSize > data_size)   //! 不够
        return 0;

    const char *str_ptr = static_cast<const char*>(unpack.fetchNoCopy(content_size));
    std::string json_text(str_ptr, content_size);

    if (is_log_enabled_)
        LogTrace("%s recv: %s", log_label_.c_str(), json_text.c_str());

    Json js;
    bool is_throw = tbox::CatchThrow([&] { js = Json::parse(json_text); });
    if (is_throw) {
        LogNotice("parse json fail");
        return -1;
    }

    onRecvJson(js);
    return unpack.pos();
}

}
}
