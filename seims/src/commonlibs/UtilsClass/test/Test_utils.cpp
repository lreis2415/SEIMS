#include "utils.h"
#include "gtest/gtest.h"

TEST(Testutils, isIPAddress) {
    const char* ip1 = "127.0.0.1";
    const char* ip2 = "192.168.6";
    const char* ip3 = "192.168.6.256";
    EXPECT_TRUE(utils::isIPAddress(ip1));
    EXPECT_FALSE(utils::isIPAddress(ip2));
    EXPECT_FALSE(utils::isIPAddress(ip3));
}
