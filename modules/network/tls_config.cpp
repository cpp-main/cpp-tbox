/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *   //  E A S Y  /  \/ \
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
#include "tls_config.h"

#include <tbox/base/log.h>

namespace tbox {
namespace network {

bool TlsConfig::isValid() const
{
    //! cert_file 和 key_file 必须同时设置或同时为空
    if (!cert_file.empty() && key_file.empty()) {
        LogErr("cert_file is set but key_file is not");
        return false;
    }
    if (!key_file.empty() && cert_file.empty()) {
        LogErr("key_file is set but cert_file is not");
        return false;
    }
    //! ca_file 与 ca_path 可选，不要求必须设置：
    //! - verify_peer=true 但未指定 ca_file/ca_path 时，Client 使用系统默认 CA
    //!   (SSL_CTX_set_default_verify_paths)，如 /etc/ssl/certs
    //! - Server 不验证客户端证书时不需要 CA
    //! - 指定了 ca_file 或 ca_path 时，两者至少有一个非空即可，OpenSSL 会正常加载

    return true;
}

}
}
