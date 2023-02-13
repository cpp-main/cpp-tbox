#include "dns_request.h"

#include <unistd.h>
#include <sstream>
#include <map>
#include <memory>

#include <tbox/base/assert.h>
#include <tbox/util/serializer.h>
#include <tbox/util/string.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace network {

namespace {

constexpr const char *HOSTS_FILE = "/etc/hosts";
constexpr const char *RESOLV_FILE = "/etc/resolv.conf";

constexpr uint16_t DNS_TYPE_A = 0x0001;
constexpr uint16_t DNS_TYPE_CNAME = 0x0005;

/// 将domain写入到缓冲
/**
 * "www.baidu.com\x0" --> "\x03www\0x5baidu\0x3com\0x0"
 */
void AppendDomain(util::Serializer &dump, const std::string domain)
{
    std::vector<std::string> str_vec;
    util::string::Split(domain, ".", str_vec);

    for (auto &seg : str_vec) {
        dump << uint8_t(seg.length());
        dump.append(seg.data(), seg.length());
    }
    dump << uint8_t(0);
}

/// 从缓冲中提取domain，与AppendDomain()相反
std::string FetchDomain(util::Deserializer &parser)
{
    std::ostringstream oss;
    bool first = true;
    for (;;) {
        uint8_t len = 0;
        parser >> len;
        if (len == 0)
            break;

        if (!first)
            oss << '.';
        first = false;

        //! 处理压缩的字串
        if ((len & 0xc0) == 0xc0) {
            uint8_t offset_low = 0;
            parser >> offset_low;
            uint16_t offset = (len & 0x3f) << 8 | offset_low;
            util::Deserializer sub_parser(parser);
            sub_parser.set_pos(offset);
            oss << FetchDomain(sub_parser);
            break;
        } else {
            char str[len + 1] = { 0 };
            parser.fetch(str, len);
            oss << str;
        }
    }
    return oss.str();
}

/// 剃除注释："xxxx #this is comment" --> "xxx "
std::string StripComment(const std::string &str)
{
    auto end_pos = str.find_first_of('#');
    if (end_pos == std::string::npos)
        return str;
    return str.substr(0, end_pos);
}

/// 从 hosts 文件中读取已有记录
bool ReadHostsFile(std::map<DomainName, IPAddress> &domain_ip_map)
{
    bool ret = util::fs::ReadEachLineFromTextFile(HOSTS_FILE,
        [&] (const std::string &line) {
            auto striped_line = StripComment(line);
            LogTrace("striped_line: %s", striped_line.c_str());
            std::vector<std::string> str_vec;
            util::string::SplitBySpace(striped_line, str_vec);
            if (str_vec.size() >= 2u) {
                try {
                    domain_ip_map[DomainName(str_vec[1])] = IPAddress::FromString(str_vec[0]);
                } catch (const std::exception &e) { }
            }
        }
    );
    return ret;
}

/// 从 resolv.conf 文件中读取DNS服务器IP地址
bool ReadResolvConfFile(std::vector<IPAddress> &dns_srv_ip_vec)
{
    bool ret = util::fs::ReadEachLineFromTextFile(RESOLV_FILE,
        [&] (const std::string &line) {
            auto striped_line = StripComment(line);
            LogTrace("striped_line: %s", striped_line.c_str());
            std::vector<std::string> str_vec;
            util::string::SplitBySpace(striped_line, str_vec);
            if (str_vec.size() >= 2u && str_vec[0] == "nameserver") {
                try {
                LogTrace("%s", str_vec[1].c_str());
                    dns_srv_ip_vec.push_back(IPAddress::FromString(str_vec[1]));
                    LogTag();
                } catch (const std::exception &e) {
                LogTag();
                }
            }
        }
    );
    return ret;
}

}

struct DnsRequest::Request {
    std::string domain_name;
    uint16_t id = 0;
    Callback cb;
    Result result;
};

DnsRequest::DnsRequest(event::Loop *wp_loop) :
    udp_(wp_loop),
    work_thread_(wp_loop)
{
    using namespace std::placeholders;
    udp_.setRecvCallback(std::bind(&DnsRequest::onUdpRecv, this, _1, _2, _3));
}

DnsRequest::~DnsRequest() {
    udp_.disable();
    CHECK_DELETE_RESET_OBJ(req_);
}

bool DnsRequest::request(const DomainName &domain, const Callback &cb) {
    if (req_ != nullptr) {
        LogWarn("already underway");
        return false;
    }

    req_ = new Request;
    TBOX_ASSERT(req_ != nullptr);

    req_->id = ++req_id_alloc_;
    req_->cb = cb;
    req_->domain_name = domain.toString();

    readHostFile();
    return true;
}

void DnsRequest::cancel() {
    udp_.disable();
    CHECK_DELETE_RESET_OBJ(req_);
}

void DnsRequest::onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from) {
    std::string hex_str = util::string::RawDataToHexStr(data_ptr, data_size);
    LogInfo("recv from %s : %s", from.toString().c_str(), hex_str.c_str());

    if (req_ == nullptr)
        return;

    util::Deserializer parser(data_ptr, data_size);

    uint16_t id, flags;
    parser >> id >> flags;
    if (id != req_->id)
        return;

    //!TODO:检查flags

    uint16_t qd_count, an_count, ns_count, ar_count;
    parser >> qd_count >> an_count >> ns_count >> ar_count;
    LogDbg("id:%d, flags:%04x, qd_count:%d, an_count:%d, ns_count:%d, ar_count:%d",
           id, flags, qd_count, an_count, an_count, ns_count, ar_count);

    //! 解析Question字段
    LogDbg("=== questions ===");
    for (uint16_t i = 0; i < qd_count; ++i) {
        std::string domain = FetchDomain(parser);
        LogInfo("domin:%s", domain.c_str());

        uint16_t dns_type, dns_class;
        parser >> dns_type >> dns_class;
        LogDbg("dns_type:%d, dns_class:%d", dns_type, dns_class);
    }

    LogDbg("=== answers ===");
    for (uint16_t i = 0; i < an_count; ++i) {
        std::string domain = FetchDomain(parser);
        LogDbg("domain:%s", domain.c_str());
        uint16_t an_type, an_class, an_len;
        uint32_t an_ttl;
        parser >> an_type >> an_class >> an_ttl >> an_len;

        LogDbg("type:%d, class:%d, ttl:%d, len:%d", an_type, an_class, an_ttl, an_len);
        if (an_type == DNS_TYPE_A) {
            uint32_t ip_value;
            auto old_endian = parser.setEndian(util::Endian::kLittle);
            parser >> ip_value;
            parser.setEndian(old_endian);
            req_->result.a_vec.emplace_back(ip_value);

        } else if (an_type == DNS_TYPE_CNAME) {
            std::string domain = FetchDomain(parser);
            req_->result.cname_vec.emplace_back(domain);

        } else {
            LogNotice("unknow type:%d", an_type);
            parser.skip(an_len);
        }
        LogDbg("-------");
    }

    if (req_->cb)
        req_->cb(req_->result);

    CHECK_DELETE_RESET_OBJ(req_);
}

void DnsRequest::readHostFile()
{
    struct Tmp {
        bool is_success = false;
        std::map<DomainName, IPAddress> domain_ip_map;
    };
    auto tmp = std::make_shared<Tmp>();
    work_thread_.execute(
        [tmp] {
            tmp->is_success = ReadHostsFile(tmp->domain_ip_map);
        },
        [tmp, this] {
            if (tmp->is_success) {
                auto iter = tmp->domain_ip_map.find(DomainName(req_->domain_name));
                if (iter != tmp->domain_ip_map.end()) {
                    req_->result.a_vec.push_back(iter->second);
                }
            }
            readResolvConfFile();
        }
    );
}

void DnsRequest::readResolvConfFile()
{
    struct Tmp {
        bool is_success = false;
        std::vector<IPAddress> dns_srv_ip_vec;
    };
    auto tmp = std::make_shared<Tmp>();

    work_thread_.execute(
        [tmp] {
            tmp->is_success = ReadResolvConfFile(tmp->dns_srv_ip_vec);
        },
        [tmp, this] {
        LogTrace("is_success:%d", tmp->is_success);
            if (tmp->is_success) {
                sendRequestTo(tmp->dns_srv_ip_vec);
            }
        }
    );
}

void DnsRequest::sendRequestTo(const IPAddressVec &dns_srv_ip_vec)
{
    std::vector<uint8_t> send_buff;
    util::Serializer dump(send_buff);

    uint16_t id = req_->id;
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

    AppendDomain(dump, req_->domain_name);

    dump << dns_type
         << dns_class;

    std::string hex_str = util::string::RawDataToHexStr(send_buff.data(), send_buff.size());

    for (auto &ip : dns_srv_ip_vec) {
        LogTag();
        SockAddr dns_srv(ip, 53);
        udp_.send(send_buff.data(), send_buff.size(), dns_srv);
        LogInfo("send to %s : %s", dns_srv.toString().c_str(), hex_str.c_str());
    }

    udp_.enable();
}

}
}
