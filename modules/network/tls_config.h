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
#ifndef TBOX_NETWORK_TLS_CONFIG_H_20260616
#define TBOX_NETWORK_TLS_CONFIG_H_20260616

#include <string>

namespace tbox {
namespace network {

//! TLS 配置结构体
struct TlsConfig {
    //! 通用配置
    std::string ca_file;        //!< CA 证书文件路径
    std::string ca_path;        //!< CA 证书目录路径

    bool verify_peer = true;    //!< 是否验证对端证书
    int  verify_depth = 1;      //!< 证书链验证深度

    //! 本端证书和私钥
    //! Server 场景：必须设置，用于向 client 出示证书
    //! Client 场景：可选设置，用于双向 TLS（mTLS）向 server 出示证书
    std::string cert_file;      //!< 本端证书文件
    std::string key_file;       //!< 本端私钥文件

    //! Client SNI 配置
    std::string hostname;       //!< 用于 SNI (Server Name Indication) 的主机名

    //! 检查配置是否有效
    bool isValid() const;
};

}
}
#endif //TBOX_NETWORK_TLS_CONFIG_H_20260616
