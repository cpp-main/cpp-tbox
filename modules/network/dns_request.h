#ifndef TBOX_NETWORK_DNS_REQUEST_H_20230207
#define TBOX_NETWORK_DNS_REQUEST_H_20230207

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/eventx/work_thread.h>
#include "udp_socket.h"
#include "domain_name.h"

namespace tbox {
namespace network {

/// DNS请求，用于发送DNS请求
class DnsRequest {
  public:
    using IPAddressVec  = std::vector<IPAddress>;

    struct A {
        uint32_t ttl;
        IPAddress ip;
    };

    struct CNAME {
        uint32_t ttl;
        DomainName cname;
    };

  public:
    explicit DnsRequest(event::Loop *wp_loop);
    explicit DnsRequest(event::Loop *wp_loop, const IPAddressVec &dns_ip_vec);
    virtual ~DnsRequest();

    //! 结果状态
    struct Result {
        enum class Status {
            kSuccess,           //!< 成功
            /// 以下是错误状态
            kDomainError,       //!< 域名错误
            kAllDnsFail,        //!< 所有服务器都失败
            kTimeout,           //!< 所有的DNS回复超时
            kFail,              //!< 其它错误
        };

        Status status = Status::kSuccess;   //!< 结果状态
        std::vector<A>      a_vec;      //!< A记录列表
        std::vector<CNAME>  cname_vec;  //!< CNAME记录列表
    };
    /// 请求结束回调
    using Callback = std::function<void(const Result &result)>;

    /// 设置DNS IP地址
    void setDnsIPAddesses(const IPAddressVec &dns_ip_vec);

    /// 向DNS服务器发送查询请求
    bool request(const DomainName &domain, const Callback &cb);

    /// 取消当前的查询请求
    void cancel();

    /// 检查当前是否处于查询中
    bool isRunning() const;

  protected:
    void init();
    void onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from);
    void onTimeout();

  private:
    UdpSocket udp_;
    event::TimerEvent *timeout_timer_ = nullptr;

    IPAddressVec dns_ip_vec_;
    uint16_t req_id_alloc_ = 0;

    struct Request;
    Request *req_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_DNS_REQUEST_H_20230207
