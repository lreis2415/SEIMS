#include "gtest/gtest.h"
#include "../../src/utils_filesystem.h"

using namespace ccgl::utils_filesystem;

TEST(TestutilsFileIO, GetAbsolutePath) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("c:\\test\\dem.tif", GetAbsolutePath("c:/test/dem.tif"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:\\test\\config.fig"));
    EXPECT_EQ("c:\\config.fig", GetAbsolutePath("c:/test/..\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:/test/\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:/test\\/config.fig"));
    EXPECT_EQ("c:\\test\\data\\config.fig", GetAbsolutePath("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("c:\\test\\file.txt", GetAbsolutePath("c:\\test/file.txt"));
    EXPECT_EQ("c:\\test\\file", GetAbsolutePath("c:\\test/file"));
#elif defined XCODE
    // the file or directory should be existed.
    string tpath = GetAppPath() + "data/delDirRecursively/txtfile.txt";
    EXPECT_EQ(tpath, GetAbsolutePath("./data/delDirRecursively/txtfile.txt"));
    EXPECT_EQ(tpath, GetAbsolutePath("./data/../data/delDirRecursively/txtfile.txt"));
#else
    // linux or unix sytle, the file or directory should be existed.
    string tpath = GetAppPath() + "data/delDirRecursively/txtfile.txt";
    // Only run succeed by `make UnitTestCoverage`
    EXPECT_EQ(tpath, GetAbsolutePath("./test/data/delDirRecursively/txtfile.txt"));
    EXPECT_EQ(tpath, GetAbsolutePath("./test/data/../data/delDirRecursively/txtfile.txt"));
#endif /* windows */
}

TEST(TestutilsFileIO, GetPathFromFullName) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:/test/dem.tif"));
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:\\test\\config.fig"));
    EXPECT_EQ("c:\\", GetPathFromFullName("c:/test/..\\config.fig"));
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:/test/\\config.fig"));
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:/test\\/config.fig"));
    EXPECT_EQ("c:\\test\\data\\", GetPathFromFullName("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:\\test/file.txt"));
    EXPECT_EQ("c:\\test\\", GetPathFromFullName("c:\\test/file"));
    EXPECT_EQ("c:\\test\\file\\", GetPathFromFullName("c:\\test/file\\"));
    EXPECT_EQ("c:\\test\\file\\", GetPathFromFullName("c:\\test/file/"));
#elif defined XCODE
    string tpath = GetAppPath() + "data/delDirRecursively/";
    EXPECT_EQ(tpath, GetPathFromFullName("./data/delDirRecursively/txtfile.txt"));
#else
    // linux or unix sytle, the file or directory should be existed.
    string tpath = GetAppPath() + "data/delDirRecursively/";
    EXPECT_EQ(tpath, GetPathFromFullName("./test/data/delDirRecursively/txtfile.txt"));
#endif /* windows */
}

TEST(TestutilsFileIO, GetSuffix) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("tif", GetSuffix("c:/test/dem.tif"));
    EXPECT_EQ("fig", GetSuffix("c:\\test\\config.fig"));
    EXPECT_EQ("fig", GetSuffix("c:/test/..\\config.fig"));
    EXPECT_EQ("fig", GetSuffix("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("txt", GetSuffix("c:\\test/file.txt"));
    EXPECT_EQ("", GetSuffix("c:\\test/file"));
#elif defined XCODE
    EXPECT_EQ("txt", GetSuffix("./data/delDirRecursively/txtfile.txt"));
#else
    // linux or unix sytle, the file or directory should be existed.
    EXPECT_EQ("txt", GetSuffix("./test/data/delDirRecursively/txtfile.txt"));
#endif /* windows */
    // without suffix
    EXPECT_EQ("", GetSuffix("abcdefg"));
}

TEST(TestutilsFileIO, GetCoreFileName) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("dem", GetCoreFileName("c:/test/dem.tif"));
    EXPECT_EQ("config", GetCoreFileName("c:\\test\\config.fig"));
    EXPECT_EQ("config", GetCoreFileName("c:/test/..\\config.fig"));
    EXPECT_EQ("config", GetCoreFileName("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("file", GetCoreFileName("c:\\test/file.txt"));
#elif defined XCODE
    EXPECT_EQ("txtfile", GetCoreFileName("./data/delDirRecursively/txtfile.txt"));
#else
    // linux or unix sytle, the file or directory should be existed.
    EXPECT_EQ("txtfile", GetCoreFileName("./test/data/delDirRecursively/txtfile.txt"));
#endif /* windows */
    // without suffix
    EXPECT_EQ("abcdefg", GetCoreFileName("abcdefg"));
}

TEST(TestutilsFileIO, DirectoryExists) {
    string testpath = GetAppPath() + "./data/delDirRecursively/blankDir";
    EXPECT_TRUE(DeleteDirectory(testpath));
    EXPECT_FALSE(DirectoryExists(testpath));
    EXPECT_TRUE(CleanDirectory(testpath));
    EXPECT_TRUE(DirectoryExists(testpath));
}
