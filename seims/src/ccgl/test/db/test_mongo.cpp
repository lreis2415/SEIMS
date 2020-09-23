#ifdef USE_MONGODB
#include "gtest/gtest.h"
#include "../../src/basic.h"
#include "../../src/db_mongoc.h"
#include "../test_global.h"

using namespace ccgl;
using namespace db_mongoc;

extern GlobalEnvironment* GlobalEnv;

TEST(MongoClientTest, initMongoDB) {
    MongoClient* client = MongoClient::Init(GlobalEnv->mongoHost.c_str(), GlobalEnv->mongoPort);
    EXPECT_NE(nullptr, client);
}
#endif /* USE_MONGODB */
