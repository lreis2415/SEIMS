#include "src/commonlibs/UtilsClass/utils.h"
#include "gtest/gtest.h"

TEST(UtilsFileIOTest, commonUse) {
    utilsFileIO utilsfileio;
    // windows path style, mixed style
    EXPECT_EQ("tif", utilsfileio.GetSuffix("c:/test/dem.tif"));
    EXPECT_EQ("fig", utilsfileio.GetSuffix("c:\\test\\config.fig"));
    EXPECT_EQ("txt", utilsfileio.GetSuffix("c:\\test/file.txt"));
    // linux or unix sytle
    EXPECT_EQ("tif", utilsfileio.GetSuffix("/usr/seims/data/dem.tif"));
    // without suffix
    EXPECT_EQ("", utilsfileio.GetSuffix("abcdefg"));
}