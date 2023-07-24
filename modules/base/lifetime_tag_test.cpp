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
#include "lifetime_tag.hpp"

namespace tbox {
namespace {

struct Host {
  LifetimeTag life;
};

TEST(LifetimeTag, LifetimeTagCopyConstruct) {
  LifetimeTag tag1;
  LifetimeTag::Watcher w1;
  LifetimeTag::Watcher w2;
  {
    w1 = tag1;
    LifetimeTag tag2(tag1);
    w2 = tag2;
    EXPECT_TRUE(w1);
    EXPECT_TRUE(w2);
  }
  EXPECT_TRUE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, LifetimeTagMoveConstruct) {
  LifetimeTag tag1;
  LifetimeTag::Watcher w1;
  LifetimeTag::Watcher w2;
  {
    w1 = tag1;
    LifetimeTag tag2(std::move(tag1));
    w2 = tag2;
    EXPECT_TRUE(w1);
    EXPECT_TRUE(w2);
  }
  EXPECT_TRUE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, WatchConstructFromLifetimeTag) {
  auto tmp = new Host;
  LifetimeTag::Watcher w(tmp->life);
  EXPECT_TRUE(w);

  delete tmp;
  EXPECT_FALSE(w);
}

TEST(LifetimeTag, WatchCopyConstruct) {
  auto tmp = new Host;
  LifetimeTag::Watcher w1(tmp->life);
  auto w2 = w1;
  EXPECT_TRUE(w1);
  EXPECT_TRUE(w2);

  delete tmp;
  EXPECT_FALSE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, WatchMoveConstruct) {
  auto tmp = new Host;
  LifetimeTag::Watcher w1(tmp->life);
  auto w2 = std::move(w1);
  EXPECT_FALSE(w1);
  EXPECT_TRUE(w2);

  delete tmp;
  EXPECT_FALSE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, WatchAssignFromLifetimeTag) {
  auto tmp = new Host;
  LifetimeTag::Watcher w;
  EXPECT_FALSE(w);

  w = tmp->life;
  EXPECT_TRUE(w);

  delete tmp;
  EXPECT_FALSE(w);
}

TEST(LifetimeTag, WatchCopyAssign) {
  auto tmp = new Host;
  LifetimeTag::Watcher w1(tmp->life);
  LifetimeTag::Watcher w2;
  EXPECT_FALSE(w2);

  w2 = w1;
  EXPECT_TRUE(w1);
  EXPECT_TRUE(w2);

  delete tmp;
  EXPECT_FALSE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, WatchMoveAssign) {
  auto tmp = new Host;
  LifetimeTag::Watcher w1(tmp->life);
  LifetimeTag::Watcher w2;
  EXPECT_FALSE(w2);

  w2 = std::move(w1);
  EXPECT_FALSE(w1);
  EXPECT_TRUE(w2);

  delete tmp;
  EXPECT_FALSE(w1);
  EXPECT_FALSE(w2);
}

TEST(LifetimeTag, LifetimeTag_Get) {
  auto tmp = new Host;
  auto w = tmp->life.get();
  EXPECT_TRUE(w);

  delete tmp;
  EXPECT_FALSE(w);
}

}
}
