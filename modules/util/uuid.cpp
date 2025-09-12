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
#include "uuid.h"

#include <random>
#include <sstream>

namespace tbox {
namespace util {

namespace {
std::string _GenByRandom()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::ostringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i)
        ss << dis(gen);
    ss << '-';
    for (int i = 0; i < 4; ++i)
        ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; ++i)
        ss << dis(gen);
    ss << '-';
    ss  << dis2(gen);
    for (int i = 0; i < 3; ++i)
        ss << dis(gen);
    ss << '-';
    for (int i = 0; i < 12; ++i)
        ss << dis(gen);

    return ss.str();
}
}

std::string GenUUID(UUIDGenStratey stratey)
{
    switch (stratey) {
        case UUIDGenStratey::kRandom:
            return _GenByRandom();
        default:
            return "";
    }
}

}
}
