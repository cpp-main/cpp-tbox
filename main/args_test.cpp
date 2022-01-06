#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include "args.h"

using namespace tbox;
using namespace tbox::main;

TEST(Args, Default)
{
    Json orig = R"(
  "a":{
    "b":12
  }
)"_json;
    auto dest = orig;

    const char *argv[] = {"test"};
    Args args(dest);
    EXPECT_TRUE(args.parse(1, argv));
    EXPECT_EQ(orig, dest);
}

TEST(Args, AddField)
{

}

TEST(Args, DeleteField)
{

}

TEST(Args, UpdateInt)
{

}

TEST(Args, UpdateString)
{

}

TEST(Args, UpdateJson)
{

}

TEST(Args, LoadConfig)
{

}

TEST(Args, LoadConfigAndUpdateField)
{

}
