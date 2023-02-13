#ifndef TBOX_NETWORK_DNS_REQUEST_H_20230207
#define TBOX_NETWORK_DNS_REQUEST_H_20230207

#include <tbox/event/loop.h>
#include <tbox/eventx/work_thread.h>
#include "udp_socket.h"
#include "domain_name.h"

namespace tbox {
namespace network {

class DnsRequest {
  public:
    explicit DnsRequest(event::Loop *wp_loop);
    virtual ~DnsRequest();

    using IPAddressVec  = std::vector<IPAddress>;
    using DomainNameVec = std::vector<DomainName>;

    //! 结果
    struct Result {
        enum class Status {
            kSuccess,           //!< 成功
            kCantReadResolvConf,//!< 读取 /etc/resolv.conf 失败
            kNoDnsServer,       //!< 没有提供DNS服务器IP地址
            kAllDnsNotRespond,  //!< 所有的DNS都没有回复
        };

        Status status = Status::kSuccess;
        IPAddressVec    a_vec;      //!< A记录列表
        DomainNameVec   cname_vec;  //!< CNAME记录列表
    };
    using Callback = std::function<void(const Result &result)>;

    bool request(const DomainName &domain, const Callback &cb);
    void cancel();

    bool isRunning() const;

  protected:
    void onUdpRecv(const void *data_ptr, size_t data_size, const SockAddr &from);

    void readHostFile();
    void readResolvConfFile();
    void sendRequestTo(const IPAddressVec &dns_srv_ip_vec);

  private:
    UdpSocket udp_;
    eventx::WorkThread work_thread_;

    uint16_t req_id_alloc_ = 0;

    struct Request;
    Request *req_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_DNS_REQUEST_H_20230207
