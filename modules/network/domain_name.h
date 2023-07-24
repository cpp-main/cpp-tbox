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
