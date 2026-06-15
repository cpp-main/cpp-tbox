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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_CRYPTO_SHA1_H_20260612
#define TBOX_CRYPTO_SHA1_H_20260612

#include <cstdint>
#include <cstddef>

namespace tbox {
namespace crypto {

/**
 * SHA-1 计算器
 *
 * 使用方法与 MD5 类似：
 *
 * SHA1 sha1;
 * sha1.update(data_ptr, data_len);
 * uint8_t digest[20];
 * sha1.finish(digest);
 *
 * 或使用一次性接口：
 * SHA1::Calc(data_ptr, data_len, digest);
 *
 * 注意：SHA-1 已不推荐用于安全目的，仅用于 WebSocket 握手等非安全场景
 */
class SHA1 {
  public:
    SHA1();

  public:
    //! 将数据喂给SHA-1，可重复调用
    void update(const void *data_ptr, size_t data_len);

    //! 结束运算，输出20字节摘要到digest
    void finish(uint8_t digest[20]);

    //! 一次性计算，便捷接口
    static void Calc(const void *data_ptr, size_t data_len, uint8_t digest[20]);

  private:
    void transform(const uint8_t block[64]);

    uint32_t state_[5];
    uint64_t count_;
    uint8_t buffer_[64];
    size_t buffer_index_;

    bool is_finished_ = false;
};

}
}

#endif //TBOX_CRYPTO_SHA1_H_20260612