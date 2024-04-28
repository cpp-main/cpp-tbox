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
#ifndef TBOX_JSONRPC_HEADER_STREAM_PROTO_H_20230812
#define TBOX_JSONRPC_HEADER_STREAM_PROTO_H_20230812

#include "../proto.h"

namespace tbox {
namespace jsonrpc {

/**
 * 含头部信息的流协议
 *
 * +--------+--------+---------------+
 * |  Head  | Length |     JSON      |
 * +--------+--------+---------------+
 * |   2B   |   4B   |     Length    |
 * +--------+--------+---------------+
 *
 * 采用 [魔幻数+长度] 的形式进行数据包界定
 *
 * 适用于流式协议，如 TCP
 */
class HeaderStreamProto : public Proto {
  public:
    explicit HeaderStreamProto(uint16_t head_code);
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) override;

  protected:
    virtual void sendJson(const Json &js) override;

  private:
    uint16_t header_code_;
};

}
}

#endif //TBOX_JSONRPC_HEADER_STREAM_PROTO_H_20230812
