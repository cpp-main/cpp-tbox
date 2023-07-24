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

#include "json.hpp"

TEST(Json, Parse)
{
    tbox::Json j = R"(
{
    "pi": 3.141,
    "happy": true,
    "name": "Niels",
    "nothing": null,
    "answer": {
        "everything": 42
    },
    "list": [1, 0, 2],
    "object": {
        "currency": "USD",
        "value": 42.99
    }
}
    )"_json;

    EXPECT_DOUBLE_EQ(j["pi"].get<double>(), 3.141);
    EXPECT_TRUE(j["happy"].get<bool>());
    EXPECT_EQ(j["name"].get<std::string>(), "Niels");
    EXPECT_EQ(j["object"]["currency"].get<std::string>(), "USD");
    EXPECT_DOUBLE_EQ(j["object"]["value"].get<double>(), 42.99);
}

TEST(Json, Stringfy)
{
    tbox::Json j;
    j["pi"] = 3.141;
    j["happy"] = true;
    j["name"] = "Niels";
    j["nothing"] = nullptr;
    j["answer"]["everything"] = 42;
    j["list"] = { 1, 0, 2 };
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };

    std::string target_str = 
R"({
    "answer": {
        "everything": 42
    },
    "happy": true,
    "list": [
        1,
        0,
        2
    ],
    "name": "Niels",
    "nothing": null,
    "object": {
        "currency": "USD",
        "value": 42.99
    },
    "pi": 3.141
})";
    EXPECT_EQ(j.dump(4), target_str);
}
