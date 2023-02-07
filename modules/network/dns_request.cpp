#include "dns_request.h"
#include <unistd.h>
#include <sstream>
#include <tbox/util/serializer.h>
#include <tbox/util/string.h>

namespace tbox {
namespace network {

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

std::string EncodeDomain(const std::string domain) {
    std::vector<std::string> str_vec;
    util::string::Split(domain, ".", str_vec);

    std::ostringstream oss;
    for (auto &seg : str_vec)
        oss << static_cast<char>(seg.length()) << seg;
    return oss.str();
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
    std::string encoded_domain = EncodeDomain(domain);

    dump << id
         << flags
         << qd_count
         << an_count
         << ns_count
         << ar_count;

    dump.append(encoded_domain.data(), encoded_domain.length() + 1);

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
}

}
}
