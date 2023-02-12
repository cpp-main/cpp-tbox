#ifndef TBOX_NETWORK_DOMAIN_NAME_H_20230211
#define TBOX_NETWORK_DOMAIN_NAME_H_20230211

namespace tbox {
namespace network {

/// 域名
class DomainName {
  public:
    explicit DomainName(const std::string &name) : name_(name) { }
    const std::string& toString() const { return name_; }

    bool operator < (const DomainName &dn) const { return name_ < dn.name_; }

  private:
    std::string name_;
};

}
}

#endif // TBOX_NETWORK_DOMAIN_NAME_H_20230211
