#ifdef USE_MONGODB
#include "gtest/gtest.h"
#include "../../src/basic.h"
#include "../../src/utils_array.h"
#include "../../src/db_mongoc.h"
#include "../test_global.h"

using namespace ccgl;
using namespace db_mongoc;
using namespace utils_array;

extern GlobalEnvironment* GlobalEnv;

TEST(MongoClientTest, initMongoDB) {
    MongoClient* client = GlobalEnv->client_;
    EXPECT_NE(nullptr, client);
}

TEST(MongoGridFS, removeFile) {
    // Save the same 1d-array as GridFs with different metadata
    string fname = "test_data_two_floats";
    float* data1d = nullptr;
    int datalength = 2;
    Initialize1DArray(datalength, data1d, -9999.f);
    data1d[0] = 2022.f;
    data1d[1] = 4.2f;
    bson_t p = BSON_INITIALIZER;
    bson_t p2 = BSON_INITIALIZER;
    bson_oid_t oid;
    bson_oid_t oid2;
    char cid[25];
    char cid2[25];
    string sid;
    string sid2;

    char* buf = reinterpret_cast<char*>(data1d);
    int buflength = datalength * sizeof(float);

    BSON_APPEND_UTF8(&p, "TEST", "false");
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(&p, "_id", &oid);
    bson_oid_to_string(&oid, cid);
    sid = cid;
    EXPECT_TRUE(GlobalEnv->gfs_->WriteStreamData(fname, buf, buflength, &p));
    BSON_APPEND_UTF8(&p2, "TEST", "true");
    bson_oid_init(&oid2, NULL);
    BSON_APPEND_OID(&p2, "_id", &oid2);
    bson_oid_to_string(&oid2, cid2);
    sid2 = cid2;
    EXPECT_TRUE(GlobalEnv->gfs_->WriteStreamData(fname, buf, buflength, &p2));

    // Remove
    STRING_MAP filter;
    filter["TEST"] = "false";
    STRING_MAP filter2;
    filter2["TEST"] = "true";
    EXPECT_TRUE(GlobalEnv->gfs_->RemoveFile(fname, NULL, filter));
    EXPECT_TRUE(GlobalEnv->gfs_->RemoveFile(fname, NULL, filter2));

    bson_destroy(&p);
    bson_destroy(&p2);
    Release1DArray(data1d);
}

#endif /* USE_MONGODB */
