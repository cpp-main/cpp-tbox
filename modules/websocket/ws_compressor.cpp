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
#include "ws_compressor.h"

#include <cstring>
#include <zlib.h>

#include <tbox/base/log.h>

namespace tbox {
namespace websocket {

//! RFC 7692 要求去除的 DEFLATE 尾部：0x00 0x00 0xFF 0xFF
static const uint8_t kDeflateTail[4] = {0x00, 0x00, 0xFF, 0xFF};

WsCompressor::WsCompressor()
{ }

WsCompressor::~WsCompressor()
{
    reset();
}

bool WsCompressor::initialize(const WsCompressionConfig &config)
{
    if (!config.isValid()) {
        LogErr("invalid compression config");
        return false;
    }

    config_ = config;
    initialized_ = true;
    return true;
}

void WsCompressor::reset()
{
    //! no_context_takeover 模式不需要持久化 zlib 上下文
    //! 每次调用 compress/decompress 时各自初始化并结束 zlib stream
    initialized_ = false;
}

//! === compress ===

std::string WsCompressor::compress(const std::string &data)
{
    return compress(data.data(), data.size());
}

std::string WsCompressor::compress(const void *data_ptr, size_t data_size)
{
    if (!initialized_ || !config_.enabled)
        return "";

    //! no_context_takeover：每次消息独立压缩
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    //! 初始化 deflate，使用 raw deflate（不写 zlib/gzip 头）
    //! window_bits 取负值表示 raw deflate，值的绝对值为窗口位数
    int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                           -config_.max_window_bits, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        LogErr("deflateInit2 fail, ret=%d", ret);
        return "";
    }

    //! 设置输入数据
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<void*>(data_ptr));
    strm.avail_in = static_cast<uInt>(data_size);

    //! 输出缓冲区：压缩后可能比原始数据更大（如随机数据），预留足够空间
    //! deflateBound 返回压缩后的最大可能大小
    size_t max_out = deflateBound(&strm, static_cast<uInt>(data_size));
    std::string output;
    output.resize(max_out);

    strm.next_out = reinterpret_cast<Bytef*>(&output[0]);
    strm.avail_out = static_cast<uInt>(max_out);

    //! 执行压缩
    ret = deflate(&strm, Z_SYNC_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        LogErr("deflate fail, ret=%d", ret);
        deflateEnd(&strm);
        return "";
    }

    //! 计算实际输出大小
    size_t out_len = max_out - strm.avail_out;

    //! 去掉 4 字节尾部 0x00 0x00 0xFF 0xFF（RFC 7692 Section 7.2.2）
    //! 只有尾部刚好是这 4 字节时才去掉
    if (out_len >= 4 &&
        memcmp(reinterpret_cast<const uint8_t*>(&output[out_len - 4]), kDeflateTail, 4) == 0) {
        out_len -= 4;
    }

    deflateEnd(&strm);

    output.resize(out_len);
    return output;
}

//! === decompress ===

std::string WsCompressor::decompress(const std::string &data)
{
    return decompress(data.data(), data.size());
}

std::string WsCompressor::decompress(const void *data_ptr, size_t data_size)
{
    if (!initialized_ || !config_.enabled)
        return "";

    //! no_context_takeover：每次消息独立解压
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    //! 初始化 inflate，使用 raw inflate（不读 zlib/gzip 头）
    //! window_bits 取负值表示 raw inflate
    int ret = inflateInit2(&strm, -config_.max_window_bits);
    if (ret != Z_OK) {
        LogErr("inflateInit2 fail, ret=%d", ret);
        return "";
    }

    //! RFC 7692 Section 7.2.2：解压前须在数据末尾加回 4 字节尾部
    //! 将原始数据 + 4字节尾部拼入一个临时缓冲区，避免修改原始数据
    size_t input_len = data_size + 4;
    std::string input;
    input.resize(input_len);
    memcpy(&input[0], data_ptr, data_size);
    memcpy(&input[data_size], kDeflateTail, 4);

    //! 输出缓冲区：预估解压后大小
    //! 解压后通常比压缩数据大，预估为输入的 4 倍，不够时动态扩容
    std::string output;
    size_t out_capacity = input_len * 4;
    if (out_capacity < 256)
        out_capacity = 256;
    output.resize(out_capacity);

    strm.next_in = reinterpret_cast<Bytef*>(&input[0]);
    strm.avail_in = static_cast<uInt>(input_len);

    size_t total_out = 0;

    do {
        strm.next_out = reinterpret_cast<Bytef*>(&output[total_out]);
        strm.avail_out = static_cast<uInt>(out_capacity - total_out);

        ret = inflate(&strm, Z_SYNC_FLUSH);

        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            LogErr("inflate fail, ret=%d", ret);
            inflateEnd(&strm);
            return "";
        }

        total_out = strm.total_out;

        //! 输出缓冲区不够大时扩容
        if (strm.avail_out == 0 && ret != Z_STREAM_END) {
            out_capacity *= 2;
            output.resize(out_capacity);
        }
    } while (ret != Z_STREAM_END && strm.avail_in > 0);

    inflateEnd(&strm);

    output.resize(total_out);
    return output;
}

}
}
