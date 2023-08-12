#include "raw_proto.h"

#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/util/json.h>

namespace tbox {
namespace jsonrpc {

void RawProto::sendJson(const Json &js)
{
    if (cbs_.send_data_cb) {
        const auto &json_text = js.dump();
        cbs_.send_data_cb(json_text.data(), json_text.size());
    }
}

ssize_t RawProto::onRecvData(const void *data_ptr, size_t data_size)
{
    const char *str_ptr = static_cast<const char*>(data_ptr);
    auto str_len = util::json::FindEndPos(str_ptr, data_size);
    if (str_len > 0) {
        Json js;
        bool is_throw = tbox::CatchThrow([&] { js = Json::parse(str_ptr, str_ptr + str_len); });
        if (is_throw)
            return -1;

        onRecvJson(js);
        return str_len;
    }
    return 0;
}

}
}
