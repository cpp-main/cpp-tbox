#include <gtest/gtest.h>
#include "net_if.h"
#include <iostream>

namespace tbox {
namespace network {

using namespace std;

TEST(NetIF, GetAll) {
    std::vector<NetIF> net_if_vec;
    EXPECT_TRUE(GetNetIF(net_if_vec));
    for (auto &item : net_if_vec) {
        cout << item.name << ',' << item.ip.toString() << ',' << item.mask.toString() << ',' << item.flags << endl;
    }
}

TEST(NetIF, GetLo) {
    std::vector<NetIF> net_if_vec;
    EXPECT_TRUE(GetNetIF("lo", net_if_vec));
    ASSERT_EQ(net_if_vec.size(), 1u);
    auto &lo = net_if_vec.at(0);
    EXPECT_EQ(lo.name, "lo");
    EXPECT_EQ(lo.ip.toString(), "127.0.0.1");
    EXPECT_EQ(lo.mask.toString(), "255.0.0.0");
}

}
}
