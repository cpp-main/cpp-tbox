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

enum DNS_TYPE {
    DNS_TYPE_A = 0x0001,
    DNS_TYPE_CNAME = 0x0005,
};

enum DNS_RC {
    DNS_RC_FORMAT_ERROR = 1,    //!< 数据包格式错误
    DNS_RC_SERVER_FAIL  = 2,    //!< 服务器的错误
    DNS_RC_NAME_ERROR   = 3,    //!< 名称不存在
    DNS_RC_NOT_IMPL     = 4,    //!< 服务器不支持查询
    DNS_RC_REFUSED      = 5,    //!< 服务器拒绝
};

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

}

//! 请求数据
struct DnsRequest::Request {
    std::string domain_name;
    Callback cb;
    uint16_t id = 0;
    size_t response_count = 0;
    Result result;
};

DnsRequest::DnsRequest(event::Loop *wp_loop, const IPAddressVec &dns_srv_ip_vec) :
    dns_srv_ip_vec_(dns_srv_ip_vec),
    udp_(wp_loop),
    timeout_timer_(wp_loop->newTimerEvent())
{
    using namespace std::placeholders;
    udp_.setRecvCallback(std::bind(&DnsRequest::onUdpRecv, this, _1, _2, _3));

    timeout_timer_->initialize(std::chrono::seconds(5), event::Event::Mode::kOneshot);
    timeout_timer_->setCallback(std::bind(&DnsRequest::onTimeout, this));
}

DnsRequest::~DnsRequest() {
    CHECK_DELETE_RESET_OBJ(timeout_timer_);
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

#if 0
    std::string hex_str = util::string::RawDataToHexStr(send_buff.data(), send_buff.size());
    LogTrace("send data: %s", hex_str.c_str());
#endif

    for (auto &ip : dns_srv_ip_vec_) {
        LogTag();
        SockAddr dns_srv(ip, 53);
        udp_.send(send_buff.data(), send_buff.size(), dns_srv);
        //LogTrace("send to: %s", ip.toString().c_str());
    }

    udp_.enable();
    timeout_timer_->enable();
    return true;
}

void DnsRequest::cancel() {
    timeout_timer_->disable();
    udp_.disable();
    CHECK_DELETE_RESET_OBJ(req_);
}

bool DnsRequest::isRunning() const {
    return req_ != nullptr;
}

void DnsRequest::onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from) {
#if 0
    std::string hex_str = util::string::RawDataToHexStr(data_ptr, data_size);
    LogTrace("recv from %s : %s", from.toString().c_str(), hex_str.c_str());
#endif

    if (req_ == nullptr)
        return;

    util::Deserializer parser(data_ptr, data_size);

    uint16_t id, flags;
    parser >> id >> flags;
    if (id != req_->id)
        return;

    if ((flags & 0x8000) == 0)   //! 必须是回复包
        return;

    //! 检查flags
    uint8_t rcode = flags & 0x000f;

    if (rcode == 0) {   //! 正常
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
    } else {
        //! 出现异常
        if (rcode == DNS_RC_NAME_ERROR) {
            //! 如果是域名自身的问题，则直接返回失败
            req_->result.status = Result::Status::kDomainError;
        } else if (rcode == DNS_RC_FORMAT_ERROR) {
            LogNotice("dns packet error");    //! 不应该发生
            req_->result.status = Result::Status::kFail;
        } else {
            //! 如果是服务器的问题，则略过当前数据，等待其它服务器的数据
            ++req_->response_count;
            if (req_->response_count < dns_srv_ip_vec_.size())
                return;
            req_->result.status = Result::Status::kAllDnsFail;
        }
    }

    if (req_->cb)
        req_->cb(req_->result);

    CHECK_DELETE_RESET_OBJ(req_);
    timeout_timer_->disable();
}

void DnsRequest::onTimeout() {
    if (req_ == nullptr)
        return;

    req_->result.status = Result::Status::kTimeout;
    if (req_->cb)
        req_->cb(req_->result);
    CHECK_DELETE_RESET_OBJ(req_);
}

}
}
