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
#ifndef TBOX_CRYPTO_MD5_H_20221218
#define TBOX_CRYPTO_MD5_H_20221218

#include <cstdint>
#include <string>

namespace tbox {
namespace crypto {

/**
 * MD5 计算器
 *
 * 使用示例：
 * const char *str1 = "cpp-tbox, C++ Treasure Box,";
 * const char *str2 = " is an event-based service application development library.";
 *
 * MD5 md5;
 * md5.update(str1, strlen(str1));
 * md5.update(str2, strlen(str2));
 *
 * uint8_t md5_digest[16];
 * md5.finish(md5_digest);
 */
class MD5 {
  public:
    MD5();

  public:
    /**
     * \brief   将明文数据喂给MD5，可重复进行
     * \param   plain_text_ptr  明文地址
     * \param   plain_text_len  明文长度
     */
    void update(const void* plain_text_ptr, size_t plain_text_len);

    /**
     * \brief   结束运算，并获取结果
     * \param   digest  16字节长的结果输出地址
     */
    void finish(uint8_t digest[16]);

  private:
    uint32_t  count_[2];
    uint32_t  state_[4];
    uint8_t   buffer_[64];

    bool is_finished_ = false;
};

}
}

#endif //TBOX_CRYPTO_MD5_H_20221218
