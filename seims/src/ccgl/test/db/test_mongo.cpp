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
    //client->Destroy(); // the MongoClient MUST not be destroyed or deleted!
    //delete client;
}
#endif /* USE_MONGODB */
