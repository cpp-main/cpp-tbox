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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_JSONRPC_PACKET_PROTO_H_20230831
#define TBOX_JSONRPC_PACKET_PROTO_H_20230831

#include "../proto.h"

namespace tbox {
namespace jsonrpc {

/**
 * 分包协议
 *
 * 约定onRecvData()中指定的内容都是一个完整的JSON字串
 * 适用于已经有分包机制的传输协议，如：UDP, MQTT, HTTP
 */
class PacketProto : public Proto {
  public:
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) override;

  protected:
    virtual void sendJson(const Json &js) override;
};

}
}

#endif //TBOX_JSONRPC_PACKET_PROTO_H_20230831
