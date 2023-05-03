#ifndef TBOX_NETWORK_DOMAIN_NAME_H_20230211
#define TBOX_NETWORK_DOMAIN_NAME_H_20230211

#include <string>

namespace tbox {
namespace network {

/// 域名
class DomainName {
  public:
    explicit DomainName() { }
    explicit DomainName(const std::string &name) : name_(name) { }
    explicit DomainName(std::string &&name) : name_(std::move(name)) { }

    inline bool isNull() const { return name_.empty(); }
    inline const std::string& toString() const { return name_; }
    inline bool operator < (const DomainName &dn) const { return name_ < dn.name_; }
    inline DomainName& operator = (const std::string &str) { name_ = str; return *this; }
    inline DomainName& operator = (std::string &&str) { name_ = std::move(str); return *this; }
    inline operator std::string () const { return name_; }

  private:
    std::string name_;
};

}
}

#endif // TBOX_NETWORK_DOMAIN_NAME_H_20230211
