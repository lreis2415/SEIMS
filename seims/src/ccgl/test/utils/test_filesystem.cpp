#include "gtest/gtest.h"
#include "../../src/utils_filesystem.h"

using namespace ccgl::utils_filesystem;

TEST(TestutilsFileIO, GetAbsolutePath) {
#ifdef WINDOWS
    // WINDOWS path style, mixed style
    EXPECT_EQ("c:\\test\\dem.tif", GetAbsolutePath("c:/test/dem.tif"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:\\test\\config.fig"));
    EXPECT_EQ("c:\\config.fig", GetAbsolutePath("c:/test/..\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:/test/\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", GetAbsolutePath("c:/test\\/config.fig"));
    EXPECT_EQ("c:\\test\\data\\config.fig", GetAbsolutePath("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("c:\\test\\file.txt", GetAbsolutePath("c:\\test/file.txt"));
    EXPECT_EQ("c:\\test\\file", GetAbsolutePath("c:\\test/file"));
#else
    // linux or unix style, the file or directory should be existed.
    string tpath = GetAppPath() + "data/delDirRecursively/txtfile.txt";
    EXPECT_EQ(tpath, GetAbsolutePath(GetAppPath() + "./data/delDirRecursively/txtfile.txt"));
    EXPECT_EQ(tpath, GetAbsolutePath(GetAppPath() + "./data/../data/delDirRecursively/txtfile.txt"));
#endif /* WINDOWS */
}

TEST(TestutilsFileIO, GetPathFromFullName) {
#ifdef WINDOWS
    // WINDOWS path style, mixed style
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
#else
    // linux or unix style, the file or directory should be existed.
    string tpath = GetAppPath() + "data/delDirRecursively/";
    EXPECT_EQ(tpath, GetPathFromFullName(tpath + "../../data/delDirRecursively/txtfile.txt"));
#endif /* WINDOWS */
}

TEST(TestutilsFileIO, GetSuffix) {
#ifdef WINDOWS
    // WINDOWS path style, mixed style
    EXPECT_EQ("tif", GetSuffix("c:/test/dem.tif"));
    EXPECT_EQ("fig", GetSuffix("c:\\test\\config.fig"));
    EXPECT_EQ("fig", GetSuffix("c:/test/..\\config.fig"));
    EXPECT_EQ("fig", GetSuffix("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("txt", GetSuffix("c:\\test/file.txt"));
    EXPECT_EQ("", GetSuffix("c:\\test/file"));
#else
    // linux or unix style, the file or directory should be existed.
    EXPECT_EQ("txt", GetSuffix(GetAppPath() + "./data/delDirRecursively/txtfile.txt"));
#endif /* WINDOWS */
    // without suffix
    EXPECT_EQ("", GetSuffix("abcdefg"));
}

TEST(TestutilsFileIO, GetCoreFileName) {
#ifdef WINDOWS
    // WINDOWS path style, mixed style
    EXPECT_EQ("dem", GetCoreFileName("c:/test/dem.tif"));
    EXPECT_EQ("config", GetCoreFileName("c:\\test\\config.fig"));
    EXPECT_EQ("config", GetCoreFileName("c:/test/..\\config.fig"));
    EXPECT_EQ("config", GetCoreFileName("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("file", GetCoreFileName("c:\\test/file.txt"));
#else
    // linux or unix style, the file or directory should be existed.
    EXPECT_EQ("txtfile", GetCoreFileName(GetAppPath() + "./data/delDirRecursively/txtfile.txt"));
#endif /* WINDOWS */
    // without suffix
    EXPECT_EQ("abcdefg", GetCoreFileName("abcdefg"));
}

TEST(TestutilsFileIO, DirectoryExists) { // only for directory path
    string testpath = GetAppPath() + "./data/delDirRecursively/blankDir";
    EXPECT_FALSE(DirectoryExists(testpath));
    EXPECT_TRUE(CleanDirectory(testpath));
    EXPECT_TRUE(DirectoryExists(testpath));
    EXPECT_TRUE(DeleteDirectory(testpath));
    EXPECT_FALSE(DirectoryExists(testpath));
    string realfile = GetAppPath() + "./data/raster/int32.tif";
    EXPECT_FALSE(DirectoryExists(realfile));
}

TEST(TestutilsFileIO, PathExists) { // directory or file paths
    string testpath = GetAppPath() + "./data/delDirRecursively/blankDir";
    EXPECT_FALSE(PathExists(testpath));
    EXPECT_TRUE(CleanDirectory(testpath));
    EXPECT_TRUE(PathExists(testpath));
    EXPECT_TRUE(DeleteDirectory(testpath));
    EXPECT_FALSE(PathExists(testpath));
    string realfile = GetAppPath() + "./data/raster/int32.tif";
    EXPECT_TRUE(PathExists(realfile));
}
