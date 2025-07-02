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
#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include "json_deep_loader.h"
#include "fs.h"
#include "json.h"

namespace tbox {
namespace util {
namespace json {

TEST(JsonDeepLoader, Load) {
    const char *filepath = "/tmp/tbox-json-deep-loader-test.json";
    const char *json_text = \
R"(
{
  "int": 123,
  "string": "hello",
  "array": [1, 2, 3]
}
)";
    Json js;

    fs::WriteStringToTextFile(filepath, json_text);
    EXPECT_NO_THROW({ js = LoadDeeply(filepath); } );

    int i_value;
    GetField(js, "int", i_value);
    EXPECT_EQ(i_value, 123);

    fs::RemoveFile(filepath);
    EXPECT_ANY_THROW({ js = Load(filepath); } );
}

TEST(JsonDeepLoader, LoadNoThrow) {
    const char *filepath = "/tmp/tbox-json-deep-loader-test.json";
    const char *json_text = \
R"(
{
  "int": 123,
  "string": "hello",
  "array": [1, 2, 3]
}
)";
    Json js;

    fs::WriteStringToTextFile(filepath, json_text);
    EXPECT_TRUE(LoadDeeply(filepath, js));

    int i_value;
    GetField(js, "int", i_value);
    EXPECT_EQ(i_value, 123);

    fs::RemoveFile(filepath);
    EXPECT_FALSE(Load(filepath, js));
}

}
}
}
