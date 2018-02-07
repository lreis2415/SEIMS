#include "utils.h"
#include "gtest/gtest.h"

TEST(TestutilsFileIO, GetAbsolutePath) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("c:\\test\\dem.tif", utilsFileIO::GetAbsolutePath("c:/test/dem.tif"));
    EXPECT_EQ("c:\\test\\config.fig", utilsFileIO::GetAbsolutePath("c:\\test\\config.fig"));
    EXPECT_EQ("c:\\config.fig", utilsFileIO::GetAbsolutePath("c:/test/..\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", utilsFileIO::GetAbsolutePath("c:/test/\\config.fig"));
    EXPECT_EQ("c:\\test\\config.fig", utilsFileIO::GetAbsolutePath("c:/test\\/config.fig"));
    EXPECT_EQ("c:\\test\\data\\config.fig", utilsFileIO::GetAbsolutePath("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("c:\\test\\file.txt", utilsFileIO::GetAbsolutePath("c:\\test/file.txt"));
    EXPECT_EQ("c:\\test\\file", utilsFileIO::GetAbsolutePath("c:\\test/file"));
#else
    // linux or unix sytle, the file or directory should be existed.
    string tpath = utilsFileIO::GetAppPath() + "txtfile.txt";
    EXPECT_EQ(tpath, utilsFileIO::GetAbsolutePath("./txtfile.txt"));
    EXPECT_EQ(tpath, utilsFileIO::GetAbsolutePath("../test/txtfile.txt"));
#endif /* windows */
}

TEST(TestutilsFileIO, GetPathFromFullName) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:/test/dem.tif"));
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:\\test\\config.fig"));
    EXPECT_EQ("c:\\", utilsFileIO::GetPathFromFullName("c:/test/..\\config.fig"));
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:/test/\\config.fig"));
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:/test\\/config.fig"));
    EXPECT_EQ("c:\\test\\data\\", utilsFileIO::GetPathFromFullName("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:\\test/file.txt"));
    EXPECT_EQ("c:\\test\\", utilsFileIO::GetPathFromFullName("c:\\test/file"));
    EXPECT_EQ("c:\\test\\file\\", utilsFileIO::GetPathFromFullName("c:\\test/file\\"));
    EXPECT_EQ("c:\\test\\file\\", utilsFileIO::GetPathFromFullName("c:\\test/file/"));
#else
    // linux or unix sytle, the file or directory should be existed.
    string tpath = utilsFileIO::GetAppPath();
    EXPECT_EQ(tpath, utilsFileIO::GetPathFromFullName("./txtfile.txt"));
    EXPECT_EQ(tpath, utilsFileIO::GetPathFromFullName("../test/txtfile.txt"));
#endif /* windows */
}

TEST(TestutilsFileIO, GetSuffix) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("tif", utilsFileIO::GetSuffix("c:/test/dem.tif"));
    EXPECT_EQ("fig", utilsFileIO::GetSuffix("c:\\test\\config.fig"));
    EXPECT_EQ("fig", utilsFileIO::GetSuffix("c:/test/..\\config.fig"));
    EXPECT_EQ("fig", utilsFileIO::GetSuffix("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("txt", utilsFileIO::GetSuffix("c:\\test/file.txt"));
    EXPECT_EQ("", utilsFileIO::GetSuffix("c:\\test/file"));
#else
    // linux or unix sytle, the file or directory should be existed.
    EXPECT_EQ("txt", utilsFileIO::GetSuffix("./txtfile.txt"));
    EXPECT_EQ("txt", utilsFileIO::GetSuffix("../test/txtfile.txt"));
#endif /* windows */
    // without suffix
    EXPECT_EQ("", utilsFileIO::GetSuffix("abcdefg"));
}

TEST(TestutilsFileIO, GetCoreFileName) {
#ifdef windows
    // windows path style, mixed style
    EXPECT_EQ("dem", utilsFileIO::GetCoreFileName("c:/test/dem.tif"));
    EXPECT_EQ("config", utilsFileIO::GetCoreFileName("c:\\test\\config.fig"));
    EXPECT_EQ("config", utilsFileIO::GetCoreFileName("c:/test/..\\config.fig"));
    EXPECT_EQ("config", utilsFileIO::GetCoreFileName("c:/test/data/..\\data/config.fig"));
    EXPECT_EQ("file", utilsFileIO::GetCoreFileName("c:\\test/file.txt"));
#else
    // linux or unix sytle, the file or directory should be existed.
    EXPECT_EQ("txtfile", utilsFileIO::GetCoreFileName("./txtfile.txt"));
    EXPECT_EQ("txtfile", utilsFileIO::GetCoreFileName("../test/txtfile.txt"));
#endif /* windows */
    // without suffix
    EXPECT_EQ("abcdefg", utilsFileIO::GetCoreFileName("abcdefg"));
}

TEST(TestutilsFileIO, DirectoryExists) {
    string testpath = utilsFileIO::GetAppPath() + "../data/delDirRecursively";
    EXPECT_TRUE(utilsFileIO::DeleteDirectory(testpath));
    EXPECT_FALSE(utilsFileIO::DirectoryExists(testpath));
    EXPECT_TRUE(utilsFileIO::CleanDirectory(testpath));
    EXPECT_TRUE(utilsFileIO::DirectoryExists(testpath));
}