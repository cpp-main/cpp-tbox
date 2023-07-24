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
#include <gtest/gtest.h>
#include "aes.h"

namespace tbox {
namespace crypto {

//! 加密解密之后结果一致
TEST(AES, CipherAndInvCipher) {
  uint8_t plaintext[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  uint8_t key[16] = {0x12, 0x33, 0x34, 0x55, 0x43}; //! Any

  AES aes(key);
  uint8_t ciphertext[16] = {0};
  aes.cipher(plaintext, ciphertext);

  uint8_t invciphertext[16] = {0};
  aes.invcipher(ciphertext, invciphertext);
  EXPECT_EQ(memcmp(plaintext, invciphertext, 16), 0);
}

}
}
