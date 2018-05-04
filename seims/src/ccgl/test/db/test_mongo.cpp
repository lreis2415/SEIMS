#ifdef USE_MONGODB
#include "gtest/gtest.h"
#include "../../src/basic.h"
#include "../../src/db_mongoc.h"

using namespace ccgl;
using namespace db_mongoc;

TEST(MongoClientTest, initMongoDB) {
    const char* ip = "127.0.0.1";
    vuint16_t port = 27017;
    MongoClient* client = MongoClient::Init(ip, port);
    EXPECT_NE(nullptr, client);
}
#endif /* USE_MONGODB */
