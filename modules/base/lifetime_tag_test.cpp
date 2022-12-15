#include <gtest/gtest.h>
#include "lifetime_tag.hpp"

namespace tbox {
namespace {

TEST(LifetimeTag, GetWatcher_Check_DeleteHost_Check) {
  struct Tmp {
    LifetimeTag tag;
  };

  auto tmp = new Tmp;
  LifetimeTag::Watcher w = tmp->tag;
  EXPECT_TRUE(w);

  delete tmp;
  EXPECT_FALSE(w);
}

}
}
