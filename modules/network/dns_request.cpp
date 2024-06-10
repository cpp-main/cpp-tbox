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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "dns_request.h"

#include <unistd.h>
#include <sstream>
#include <map>
#include <memory>

#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/util/serializer.h>
#include <tbox/util/string.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace network {

namespace {

enum DNS_TYPE {
    DNS_TYPE_A      = 1,    //!< 期望获得查询名的IP地址
    DNS_TYPE_NS     = 2,    //!< 一个授权的域名服务器
    DNS_TYPE_CNAME  = 5,    //!< 规范名称
    DNS_TYPE_PTR    = 12,   //!< 指针记录
    DNS_TYPE_HINFO  = 13,   //!< 主机信息
    DNS_TYPE_MX     = 15,   //!< 邮件交换记录
    DNS_TYPE_AXFR   = 252,  //!< 对区域转换的请求
    DNS_TYPE_ANY    = 255,  //!< 对所有记录的请求
};

enum DNS_CLASS {
    DNS_CLASS_IN    = 1,    //!< 互联网地址
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
            char str[len + 1];
            parser.fetch(str, len);
            str[len] = '\0';
            oss << str;
        }
    }
    return oss.str();
}

}

DnsRequest::DnsRequest(event::Loop *wp_loop) :
    udp_(wp_loop),
    timeout_monitor_(wp_loop)
{
    init();
}

DnsRequest::DnsRequest(event::Loop *wp_loop, const IPAddressVec &dns_ip_vec) :
    udp_(wp_loop),
    timeout_monitor_(wp_loop),
    dns_ip_vec_(dns_ip_vec)
{
    init();
}

DnsRequest::~DnsRequest() {
    udp_.disable();
}

void DnsRequest::init()
{
    using namespace std::placeholders;
    udp_.setRecvCallback(std::bind(&DnsRequest::onUdpRecv, this, _1, _2, _3));

    timeout_monitor_.initialize(std::chrono::seconds(1), 5);
    timeout_monitor_.setCallback(std::bind(&DnsRequest::onRequestTimeout, this, _1));
}

void DnsRequest::setDnsIPAddresses(const IPAddressVec &dns_ip_vec)
{
    dns_ip_vec_ = dns_ip_vec;
}

DnsRequest::ReqId DnsRequest::request(const DomainName &domain, const Callback &cb)
{
    if (dns_ip_vec_.empty()) {
        LogWarn("dns srv ip not specify");
        return false;
    }

    ReqId req_id = ++req_id_alloc_;
    std::vector<uint8_t> send_buff;

    util::Serializer dump(send_buff);

    uint16_t flags = 0x0100; //!< QR:请求, OPCODE:标准查询, RA:期望递归
    uint16_t qd_count = 1;
    uint16_t an_count = 0;
    uint16_t ns_count = 0;
    uint16_t ar_count = 0;

    dump << req_id
         << flags
         << qd_count
         << an_count
         << ns_count
         << ar_count;

    AppendDomain(dump, domain.toString());

    uint16_t dns_type  = DNS_TYPE_A;
    uint16_t dns_class = DNS_CLASS_IN;

    dump << dns_type
         << dns_class;

#if 0
    std::string hex_str = util::string::RawDataToHexStr(send_buff.data(), send_buff.size());
    LogTrace("send data: %s", hex_str.c_str());
#endif

    for (auto &ip : dns_ip_vec_) {
        SockAddr dns_srv(ip, 53);
        udp_.send(send_buff.data(), send_buff.size(), dns_srv);
    }

    addRequest(req_id, cb);
    return req_id;
}

bool DnsRequest::cancel(ReqId req_id)
{
    return deleteRequest(req_id);
}

bool DnsRequest::isRunning(ReqId req_id) const
{
    auto iter = requests_.find(req_id);
    return iter != requests_.end();
}

void DnsRequest::onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from)
{
#if 0
    std::string hex_str = util::string::RawDataToHexStr(data_ptr, data_size);
    LogTrace("recv from %s : %s", from.toString().c_str(), hex_str.c_str());
#endif

    if (requests_.empty())
        return;

    RECORD_SCOPE();
    util::Deserializer parser(data_ptr, data_size);

    uint16_t req_id, flags;
    parser >> req_id >> flags;

    Request *req = findRequest(req_id);
    if (req == nullptr)
        return;

    if ((flags & 0x8000) == 0)   //! 必须是回复包
        return;

    //! 检查flags
    uint8_t rcode = flags & 0x000f;

    Result result;

    if (rcode == 0) {   //! 正常
        uint16_t qd_count, an_count, ns_count, ar_count;
        parser >> qd_count >> an_count >> ns_count >> ar_count;

#if 0
        LogTrace("id:%d, flags:%04x, qd_count:%d, an_count:%d, ns_count:%d, ar_count:%d",
               id, flags, qd_count, an_count, an_count, ns_count, ar_count);
#endif

        //! 解析Question字段
        for (uint16_t i = 0; i < qd_count; ++i) {
            FetchDomain(parser);
            uint16_t dns_type, dns_class;
            parser >> dns_type >> dns_class;
        }

        for (uint16_t i = 0; i < an_count; ++i) {
            FetchDomain(parser);
            uint16_t an_type, an_class, an_len;
            uint32_t an_ttl;
            parser >> an_type >> an_class >> an_ttl >> an_len;

#if 0
            LogTrace("type:%d, class:%d, ttl:%d, len:%d", an_type, an_class, an_ttl, an_len);
#endif
            if (an_type == DNS_TYPE_A) {
                uint32_t ip_value;
                auto old_endian = parser.setEndian(util::Endian::kLittle);
                parser >> ip_value;
                parser.setEndian(old_endian);
                A a = { an_ttl, IPAddress(ip_value) };
                result.a_vec.push_back(a);

            } else if (an_type == DNS_TYPE_CNAME) {
                std::string domain = FetchDomain(parser);
                CNAME cname = { an_ttl, DomainName(domain) };
                result.cname_vec.push_back(cname);

            } else {
                LogNotice("unknow type:%d", an_type);
                parser.skip(an_len);
            }
        }
    } else {
        //! 出现异常
        if (rcode == DNS_RC_NAME_ERROR) {
            //! 如果是域名自身的问题，则直接返回失败
            result.status = Result::Status::kDomainError;
        } else if (rcode == DNS_RC_FORMAT_ERROR) {
            LogNotice("dns packet error");    //! 不应该发生
            result.status = Result::Status::kFail;
        } else {
            //! 如果是服务器的问题，则略过当前数据，等待其它服务器的数据
            ++req->response_count;
            if (req->response_count < dns_ip_vec_.size())
                return;
            result.status = Result::Status::kAllDnsFail;
        }
    }

    if (req->cb)
        req->cb(result);

    deleteRequest(req_id);
    (void)from;
}

void DnsRequest::onRequestTimeout(ReqId req_id)
{
    auto req = findRequest(req_id);
    if (req == nullptr)
        return;

    Result result;
    result.status = Result::Status::kTimeout;

    if (req->cb)
        req->cb(result);

    deleteRequest(req_id);
}

void DnsRequest::addRequest(ReqId req_id, const Callback &cb)
{
    if (requests_.empty())
        udp_.enable();

    Request req;
    req.cb = cb;

    requests_[req_id] = req;
    timeout_monitor_.add(req_id);
}

DnsRequest::Request* DnsRequest::findRequest(ReqId req_id)
{
    auto iter = requests_.find(req_id);
    if (iter != requests_.end())
        return &(iter->second);
    return nullptr;
}

bool DnsRequest::deleteRequest(ReqId req_id)
{
    auto iter = requests_.find(req_id);
    if (iter != requests_.end()) {
        requests_.erase(iter);
        if (requests_.empty())
            udp_.disable();
        return true;
    }
    return false;
}

}
}
