#include "gtest/gtest.h"
#include "MongoUtil.h"

TEST(MongoClientTest, initMongoDB) {
    const char* ip = "127.0.0.1";
    uint16_t port = 27017;
    MongoClient *client = MongoClient::Init(ip, port);
    EXPECT_NE(nullptr, client);
}