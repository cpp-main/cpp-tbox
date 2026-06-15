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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_WS_FRAME_PARSER_H_20260612
#define TBOX_WS_FRAME_PARSER_H_20260612

#include "ws_frame.h"
#include <cstdint>

namespace tbox {
namespace websocket {

//! WebSocket 增量帧解析器（RFC 6455）
//! 适用于事件驱动场景，逐步从缓冲区中解析帧
class WsFrameParser {
  public:
    //! 解析状态
    enum class State {
        kInit,              //!< 初始状态，等待新帧
        kHeader2Bytes,      //!< 已读取首2字节，等待剩余头部
        kPayloadLen16,      //!< 等待16位扩展长度
        kPayloadLen64,      //!< 等待64位扩展长度
        kMaskKey,           //!< 等待4字节掩码
        kPayload,           //!< 等待负载数据
        kFinished,          //!< 一帧解析完成
        kError,             //!< 解析出错
    };

    WsFrameParser();

    //! 从数据中解析，返回已消费的字节数
    size_t parse(const void *data_ptr, size_t data_size);

    //! 获取当前状态
    State state() const { return state_; }

    //! 获取解析完成的帧（仅 state == kFinished 时有效）
    //! 取走后，解析器自动重置为 kInit
    WsFrame* getFrame();

    //! 重置解析器
    void reset();

  private:
    State state_ = State::kInit;

    //! 当前帧的头部信息
    bool    fin_;
    uint8_t opcode_;
    bool    masked_;
    uint64_t payload_len_;
    uint8_t mask_key_[4];

    //! 已接收的负载数据
    std::string payload_;
    uint64_t payload_received_;

    //! 解析完成的帧
    WsFrame *sp_frame_ = nullptr;
};

}
}

#endif //TBOX_WS_FRAME_PARSER_H_20260612
