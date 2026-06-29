/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S E  /  \/ \
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
#ifndef TBOX_NETWORK_TLS_FACTORY_ENTRY_H_20260626
#define TBOX_NETWORK_TLS_FACTORY_ENTRY_H_20260626

#include "tls_config.h"
#include "tcp_factory.h"

namespace tbox {
namespace network {

//! TLS 角色
enum class TlsRole {
    kClient,    //!< 作为 TLS Client（用于 TcpClient）
    kServer,    //!< 作为 TLS Server（用于 TcpServer）
};

//! TLS 工厂创建入口函数
//! 默认为弱实现（返回 nullptr），由 network_tls 模块提供强实现
//! 当链接了 libtbox_network_tls 时，强符号覆盖弱符号，TLS 功能可用
//! 当未链接 network_tls 时，弱符号生效，调用将返回 nullptr 并打印警告
extern TcpFactory* CreateTlsFactory(TlsRole role, const TlsConfig &config);

}
}
#endif //TBOX_NETWORK_TLS_FACTORY_ENTRY_H_20260626
