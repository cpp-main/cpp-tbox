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
#ifndef TBOX_TERMINAL_SESSION_IMP_H_20220204
#define TBOX_TERMINAL_SESSION_IMP_H_20220204

#include <deque>

#include "key_event_scanner.h"
#include "../session.h"

namespace tbox {
namespace terminal {

struct SessionContext {
    Connection *wp_conn = nullptr;
    SessionToken token;

    uint32_t options = 0;

    std::string curr_input;
    size_t cursor = 0;

    Path path;  //! 当前路径
    std::deque<std::string> history;   //! 历史命令
    size_t history_index = 0;   //! 0表示不指定历史命令

    KeyEventScanner key_event_scanner_;

    uint16_t window_width = 0;
    uint16_t window_height = 0;
};

}
}

#endif //TBOX_TERMINAL_SESSION_IMP_H_20220204
