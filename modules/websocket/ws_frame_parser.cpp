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
#include "ws_frame_parser.h"

#include <cstring>
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace websocket {

WsFrameParser::WsFrameParser()
{
    state_ = State::kInit;
    payload_received_ = 0;
}

size_t WsFrameParser::parse(const void *data_ptr, size_t data_size)
{
    if (state_ == State::kError || state_ == State::kFinished || data_ptr == nullptr)
        return 0;

    const uint8_t *p = static_cast<const uint8_t*>(data_ptr);
    size_t remaining = data_size;
    size_t consumed = 0;

    while (remaining > 0) {
        switch (state_) {
            case State::kInit: {
                //! 第1字节：FIN + RSV1-3 + Opcode
                fin_ = (p[0] >> 7) & 1;
                rsv1_ = (p[0] >> 6) & 1;

                //! 检查：RSV2/RSV3 必须为0（目前仅支持 RSV1 用于 permessage-deflate）
                if ((p[0] & 0x30) != 0) {
                    state_ = State::kError;
                    return consumed;
                }

                opcode_ = p[0] & 0x0F;

                ++p; --remaining; ++consumed;
                state_ = State::kHeader2Bytes;
                break;
            }

            case State::kHeader2Bytes: {
                //! 第2字节：MASK + Payload length (7 bits)
                //! RFC 6455 Section 5.2：len7 0~125 为 7-bit 长度，126 为 16-bit，127 为 64-bit
                masked_ = (p[0] >> 7) & 1;
                uint8_t len7 = p[0] & 0x7F;

                if (len7 <= 125) {
                    payload_len_ = len7;
                    ++p; --remaining; ++consumed;
                    state_ = masked_ ? State::kMaskKey : State::kPayload;
                    payload_.clear();
                    payload_received_ = 0;
                } else if (len7 == 126) {
                    payload_len_ = 0;  //! 待读取16位长度
                    ++p; --remaining; ++consumed;
                    state_ = State::kPayloadLen16;
                } else {  //! len7 == 127
                    payload_len_ = 0;  //! 待读取64位长度
                    ++p; --remaining; ++consumed;
                    state_ = State::kPayloadLen64;
                }
                break;
            }

            case State::kPayloadLen16: {
                //! 需要2字节
                if (remaining < 2)
                    return consumed;

                payload_len_ = (static_cast<uint64_t>(p[0]) << 8)
                              | static_cast<uint64_t>(p[1]);
                p += 2; remaining -= 2; consumed += 2;

                //! RFC 6455 Section 5.2：16位扩展长度必须 >= 126
                if (payload_len_ <= 125) {
                    state_ = State::kError;
                    return consumed;
                }

                state_ = masked_ ? State::kMaskKey : State::kPayload;
                payload_.clear();
                payload_received_ = 0;
                break;
            }

            case State::kPayloadLen64: {
                //! 需要8字节
                if (remaining < 8)
                    return consumed;

                //! 64位长度，大端序
                uint64_t len = 0;
                for (int i = 0; i < 8; ++i)
                    len = (len << 8) | static_cast<uint64_t>(p[i]);

                //! 最高位必须为0
                if (len >= (1ULL << 63)) {
                    state_ = State::kError;
                    return consumed;
                }

                payload_len_ = len;
                p += 8; remaining -= 8; consumed += 8;

                //! 64位长度必须 > 65535
                if (payload_len_ <= 65535) {
                    state_ = State::kError;
                    return consumed;
                }

                state_ = masked_ ? State::kMaskKey : State::kPayload;
                payload_.clear();
                payload_received_ = 0;
                break;
            }

            case State::kMaskKey: {
                //! 需要4字节掩码
                if (remaining < 4)
                    return consumed;

                memcpy(mask_key_, p, 4);
                p += 4; remaining -= 4; consumed += 4;
                state_ = State::kPayload;
                break;
            }

            case State::kPayload: {
                //! 读取负载数据
                uint64_t need = payload_len_ - payload_received_;
                size_t copy_len = (need > remaining) ? remaining : static_cast<size_t>(need);

                if (masked_) {
                    //! 解掩码：payload[i] ^= mask_key[(i + payload_received_) % 4]
                    for (size_t i = 0; i < copy_len; ++i) {
                        uint8_t byte = p[i] ^ mask_key_[(payload_received_ + i) % 4];
                        payload_.push_back(byte);
                    }
                } else {
                    payload_.append(reinterpret_cast<const char*>(p), copy_len);
                }

                payload_received_ += copy_len;
                p += copy_len; remaining -= copy_len; consumed += copy_len;

                if (payload_received_ == payload_len_) {
                    //! 帧完整，创建 WsFrame
                    sp_frame_ = new WsFrame;
                    sp_frame_->fin = fin_;
                    sp_frame_->rsv1 = rsv1_;
                    sp_frame_->opcode = static_cast<WsFrame::OpCode>(opcode_);
                    sp_frame_->payload = std::move(payload_);
                    state_ = State::kFinished;
                    return consumed;
                }
                break;
            }

            case State::kFinished:
            case State::kError:
                return consumed;
        }
    }

    return consumed;
}

WsFrame* WsFrameParser::getFrame()
{
    if (state_ != State::kFinished)
        return nullptr;

    WsFrame *frame = sp_frame_;
    sp_frame_ = nullptr;
    state_ = State::kInit;
    payload_.clear();
    payload_received_ = 0;
    return frame;
}

void WsFrameParser::reset()
{
    CHECK_DELETE_RESET_OBJ(sp_frame_);
    state_ = State::kInit;
    payload_.clear();
    payload_received_ = 0;
}

}
}
