#include "../../src/basic.h"
#include "gtest/gtest.h"

using namespace ccgl;

TEST(Testutils, isIPAddress) {
    const char* ip1 = "127.0.0.1";
    const char* ip2 = "192.168.6";
    const char* ip3 = "192.168.6.256";
    EXPECT_TRUE(IsIpAddress(ip1));
    EXPECT_FALSE(IsIpAddress(ip2));
    EXPECT_FALSE(IsIpAddress(ip3));
}

TEST(TestModelException, throwException) {
    EXPECT_THROW(throw ModelException("ModuleName", "FunctionName", "ExceptionDescription"),
        ModelException);
    string modelname = "ModuleName";
    string funcname = "FunctionName";
    string spec = "Specific information";
    string desc = "Error: " + spec;
    EXPECT_THROW(throw ModelException(modelname, funcname, desc), ModelException);
}
