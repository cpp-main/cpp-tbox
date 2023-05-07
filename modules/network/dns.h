#ifndef TBOX_NETWORK_DNS_H_20230507
#define TBOX_NETWORK_DNS_H_20230507

#include <map>
#include "dns_request.h"

namespace tbox {
namespace network {

/// DNS
///
/// 用于向其它模块提供IP地址查询服务

class Dns {
  public:
    explicit DnsRequest(event::Loop *wp_loop);
    explicit DnsRequest(event::Loop *wp_loop, const IPAddressVec &dns_ip_vec);
    virtual ~DnsRequest();

  private:
    DnsRequest dns_req_;    //!< DNS请求器

    std::map<DomainName, IPAddress> ip_address_cache_;
    struct QueryInfo {
        DnsRequest::ReqId req_id;
        std::vector<QueryCallback>
    };
    std::map<DomainName, IPAddress> ip_address_cache_;
    std::map<DomainName, QueryInfo> query_info_;
};

}
}

#endif //TBOX_NETWORK_DNS_H_20230507
