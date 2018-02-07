#include "ModelException.h"
#include "gtest/gtest.h"

TEST(TestModelException, throwException) {
    EXPECT_THROW(throw ModelException("ModuleName", "FunctionName", "ExceptionDescription"),
                 ModelException);
    string modelname = "ModuleName";
    string funcname = "FunctionName";
    string spec = "Specific information";
    string desc = "Error: " + spec;
    EXPECT_THROW(throw ModelException(modelname, funcname, desc), ModelException);
}