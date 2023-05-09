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
    using QueryId = uint32_t;

    class QueryToken {
        friend Dns;

      public:
        inline bool isNull() const { id_ == 0; }
        inline QueryId id() const { return id_; }
        inline const DomainName& domain_name() const { return domain_name_; }
        inline void reset() {
            domain_name_.clear();
            id_ = 0;
            pos_ = 0;
        }
        inline void swap(QueryToken &other) {
            std::swap(domain_name_, other.domain_name_);
            std::swap(id_, other.id_);
            std::swap(pos_, other.pos_);
        }

      private:
        DomainName domain_name_;
        QueryId id_ = 0;
        size_t pos_ = 0;
    };

    using QueryCallback = std::function<const QueryToken &, bool, const IPAddress &>;

  public:
    explicit Dns(event::Loop *wp_loop);
    virtual ~Dns();

  public:
    bool initialize(const IPAddressVec &dns_ip_vec);
    void cleanup();

  public:
    /// 同步查询，只在 cache 里查，里面没有就返回空
    IPAddress query(const DomainName &domain_name) const;

    /// 异步查询，先在 cache 里查，如果没有，则发起 DNS 请求，等有了结果再回调
    QueryToken query(const DomainName &domain_name, QueryCallback &&cb);
    /// 取消异步查询
    bool cancel(const QueryToken &token);

  protected:
    struct Lifetime {
        uint64_t ts;
        DomainName domain_name;
    };

    struct QueryInfo {
        DnsRequest::ReqId req_id;
        std::vector<QueryCallback> cb_vec_;
    };

  private:
    event::TimerEvent *tick_timer_;
    DnsRequest dns_req_;    //!< DNS请求器
    std::map<DomainName, IPAddress> cache_;
    std::map<DomainName, QueryInfo> querys_;
    std::vector<Lifetime> lifetime_min_heap_;
};

}
}

#endif //TBOX_NETWORK_DNS_H_20230507
