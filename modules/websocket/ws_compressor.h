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
 * in the LICENSE file in the root of the project source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_WS_COMPRESSOR_H_20260708
#define TBOX_WS_COMPRESSOR_H_20260708

#include <cstdint>
#include <string>

namespace tbox {
namespace websocket {

//! WebSocket 压缩配置（RFC 7692 permessage-deflate）
struct WsCompressionConfig {
    bool enabled = false;                  //!< 是否启用压缩
    bool no_context_takeover = true;       //!< 是否不保留压缩上下文（每次消息独立）
    int  max_window_bits = 15;             //!< 最大窗口位数 (8~15)

    //! 检查配置是否有效
    bool isValid() const {
        if (!enabled)
            return true;
        if (max_window_bits < 8 || max_window_bits > 15)
            return false;
        return true;
    }
};

//! WebSocket 帧压缩/解压缩器（RFC 7692 permessage-deflate）
//!
//! RFC 7692 关键规则：
//! - 使用 DEFLATE (zlib)，压缩后数据须去掉 4 字节尾 0x00 0x00 0xFF 0xFF
//! - 解压前须将 4 字节尾加回
//! - 控制帧（Close/Ping/Pong）永远不压缩
//! - no_context_takeover=true 时，每条消息独立压缩/解压（不跨消息保持 zlib 上下文）
class WsCompressor {
  public:
    WsCompressor();
    ~WsCompressor();

    //! 初始化压缩器（须在使用前调用）
    bool initialize(const WsCompressionConfig &config);

    //! 重置压缩器内部状态
    void reset();

    //! 压缩数据（去掉 4 字节尾 0x00 0x00 0xFF 0xFF）
    //! 成功返回压缩后数据，失败返回空字符串
    //! 控制帧不应调用此方法
    std::string compress(const std::string &data);
    //! 直接接受原始指针与长度，避免二进制数据构造 std::string 的额外拷贝
    std::string compress(const void *data_ptr, size_t data_size);

    //! 解压数据（先加回 4 字节尾再解压）
    //! 成功返回解压后数据，失败返回空字符串
    //! 仅对 RSV1=1 的数据帧调用此方法
    std::string decompress(const std::string &data);
    //! 直接接受原始指针与长度，避免二进制数据构造 std::string 的额外拷贝
    std::string decompress(const void *data_ptr, size_t data_size);

    //! 是否已初始化
    bool isInitialized() const { return initialized_; }

    //! 获取配置
    const WsCompressionConfig& config() const { return config_; }

  private:
    WsCompressionConfig config_;
    bool initialized_ = false;
};

}
}

#endif //TBOX_WS_COMPRESSOR_H_20260708
