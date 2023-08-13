#include "header_proto.h"

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/util/json.h>
#include <tbox/util/serializer.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace jsonrpc {

const uint16_t kHeadMagic = 0xCAFE;
const uint16_t kHeadSize = 6;   //! HeadMagic(2) + ContentLength(4)

void HeaderProto::sendJson(const Json &js)
{
    const auto &json_text = js.dump();

    std::vector<uint8_t> buff;
    util::Serializer pack(buff);

    pack << kHeadMagic << static_cast<uint32_t>(json_text.size());
    pack.append(json_text.data(), json_text.size());

    if (cbs_.send_data_cb)
        cbs_.send_data_cb(buff.data(), buff.size());
}

ssize_t HeaderProto::onRecvData(const void *data_ptr, size_t data_size)
{
    TBOX_ASSERT(data_ptr != nullptr);

    if (data_size < kHeadSize)
        return 0;

    util::Deserializer unpack(data_ptr, data_size);

    uint16_t header_magic = 0;
    uint32_t content_size = 0;
    unpack >> header_magic >> content_size; 

    if (header_magic != kHeadMagic) {
        LogNotice("head magic mismatch");
        return -2;
    }

    if (content_size + kHeadSize > data_size)   //! 不够
        return 0;

    const char *str_ptr = static_cast<const char*>(unpack.fetchNoCopy(content_size));
    Json js;
    bool is_throw = tbox::CatchThrow([&] { js = Json::parse(str_ptr, str_ptr + content_size); });
    if (is_throw)
        return -1;

    onRecvJson(js);
    return unpack.pos();
}

}
}
