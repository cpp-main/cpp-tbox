#include "dns_request.h"
#include <unistd.h>
#include <sstream>
#include <tbox/util/serializer.h>
#include <tbox/util/string.h>

namespace tbox {
namespace network {

constexpr uint16_t DNS_TYPE_A = 0x0001;
constexpr uint16_t DNS_TYPE_CNAME = 0x0005;

DnsRequest::DnsRequest(event::Loop *wp_loop) :
    udp_(wp_loop)
{
    using namespace std::placeholders;
    udp_.setRecvCallback(std::bind(&DnsRequest::onUdpRecv, this, _1, _2, _3));
    udp_.enable();
}

DnsRequest::~DnsRequest() {
    udp_.disable();
}

namespace {
void AppendDomain(util::Serializer &dump, const std::string domain) {
    std::vector<std::string> str_vec;
    util::string::Split(domain, ".", str_vec);

    for (auto &seg : str_vec) {
        dump << uint8_t(seg.length());
        dump.append(seg.data(), seg.length());
    }
    dump << uint8_t(0);
}

std::string FetchDomain(util::Deserializer &parser, const uint8_t *start_ptr) {
    std::ostringstream oss;
    bool first = true;
    for (;;) {
        if (!first) oss << '.';
        first = false;

        uint8_t len;
        parser >> len;
        if (len == 0) {
            break;
        } else if ((len & 0xc0) == 0xc0) {
            uint8_t offset_low;
            parser >> offset_low;
            uint16_t offset = (len & 0x3f) << 8 | offset_low;
            util::Deserializer sub_parser(start_ptr + offset);
            oss << FetchDomain(sub_parser, start_ptr);
            break;
        } else {
            char str[len + 1] = { 0 };
            parser.fetch(str, len);
            oss << str;
        }
    }
    return oss.str();
}
}

bool DnsRequest::request(const std::string &domain) {
    std::vector<uint8_t> send_buff;
    util::Serializer dump(send_buff);
    
    uint16_t id = ::getpid();
    uint16_t flags = 0x0100;
    uint16_t qd_count = 1;
    uint16_t an_count = 0;
    uint16_t ns_count = 0;
    uint16_t ar_count = 0;
    uint16_t dns_type = 1;
    uint16_t dns_class = 1;

    dump << id
         << flags
         << qd_count
         << an_count
         << ns_count
         << ar_count;

    AppendDomain(dump, domain);

    dump << dns_type
         << dns_class;

    SockAddr dns_srv(IPAddress("114.114.114.114"), 53);
    std::string hex_str = util::string::RawDataToHexStr(send_buff.data(), send_buff.size());
    LogInfo("send to %s : %s", dns_srv.toString().c_str(), hex_str.c_str());

    udp_.send(send_buff.data(), send_buff.size(), dns_srv);
    return true;
}

void DnsRequest::onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from) {
    std::string hex_str = util::string::RawDataToHexStr(data_ptr, data_size);
    LogInfo("recv from %s : %s", from.toString().c_str(), hex_str.c_str());
    util::Deserializer parser(data_ptr, data_size);

    uint16_t id, flags;
    uint16_t qd_count, an_count, ns_count, ar_count;

    parser >> id >> flags >> qd_count >> an_count >> ns_count >> ar_count;
    LogDbg("id:%d, flags:%04x, qd_count:%d, an_count:%d, ns_count:%d, ar_count:%d",
           id, flags, qd_count, an_count, an_count, ns_count, ar_count);

    const uint8_t *byte_ptr = static_cast<const uint8_t*>(data_ptr);
    //! 解析Question字段
    LogDbg("=== questions ===");
    for (uint16_t i = 0; i < qd_count; ++i) {
        std::string domain = FetchDomain(parser, byte_ptr);
        LogInfo("domin:%s", domain.c_str());

        uint16_t dns_type, dns_class;
        parser >> dns_type >> dns_class;
        LogDbg("dns_type:%d, dns_class:%d", dns_type, dns_class);
    }

    LogDbg("=== answers ===");
    for (uint16_t i = 0; i < an_count; ++i) {
        std::string domain = FetchDomain(parser, byte_ptr);
        LogDbg("domain:%s", domain.c_str());
        uint16_t an_type, an_class, an_length;
        uint32_t an_ttl;
        parser >> an_type >> an_class >> an_ttl >> an_length;

        LogDbg("type:%d, class:%d, ttl:%d", an_type, an_class, an_ttl);
        if (an_type == DNS_TYPE_A) {
            uint32_t ip_value;
            parser >> ip_value;
            IPAddress ip(ip_value);
            LogDbg("A:%s", ip.toString().c_str());
        } else if (an_type == DNS_TYPE_CNAME) {
            std::string domain = FetchDomain(parser, byte_ptr);
            LogDbg("CNAME:%s", domain.c_str());
        }
        LogDbg("--------");
    }
}

}
}
