/*!
* @brief Test description:
*                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
*        Raster data:      YES           YES            NO               YES
*        Mask data  :      YES            --            NO               YES
*
*        TEST CASE NAME (or TEST SUITE):
*            clsRasterDataTestWithDefaultValue
*
*        Since we mainly support ASC and GDAL(e.g., TIFF),
*        value-parameterized tests of Google Test will be used.
* @cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
* @version 1.0
* @authors Liangjun Zhu (zlj@lreis.ac.cn)
* @revised 12/07/2017 lj Initial version.
*
*/
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

using namespace std;

namespace {
#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string corename = "luid";
string maskcorename = "mask1";
string asc_file = apppath + "../data/" + corename + ".asc";
string tif_file = apppath + "../data/" + corename + ".tif";
string mask_asc_file = apppath + "../data/" + maskcorename + ".asc";
string mask_tif_file = apppath + "../data/" + maskcorename + ".tif";

struct inputRasterFiles {
public:
    inputRasterFiles(const string &rsf, const string &maskf) {
        raster_name = rsf.c_str();
        mask_name = maskf.c_str();
    };
    const char *raster_name;
    const char *mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestWithDefaultValue : public TestWithParam<inputRasterFiles *> {
public:
    clsRasterDataTestWithDefaultValue() : rs(nullptr), maskrs(nullptr) {}
    virtual ~clsRasterDataTestWithDefaultValue() { delete rs; }
    virtual void SetUp() {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs = clsRasterData<int>::Init(GetParam()->mask_name);
        ASSERT_NE(nullptr, maskrs);
        // Read raster data with the masked data
        rs = clsRasterData<int, int>::Init(GetParam()->raster_name, true, maskrs, true, 4);
        ASSERT_NE(nullptr, rs);
    }
    virtual void TearDown() {
        delete rs;
        delete maskrs;
        rs = nullptr;
        maskrs = nullptr;
    }
protected:
    clsRasterData<int, int> *rs;
    clsRasterData<int> *maskrs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestWithDefaultValue, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(73, rs->getDataLength());  // m_nCells, which is the same as the extent of mask data
    EXPECT_EQ(73, rs->getCellNumber());  // m_nCells

    EXPECT_EQ(-2147483647, rs->getNoDataValue());  // m_noDataValue
    EXPECT_EQ(4, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_FALSE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_TRUE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(rs->validate_raster_data());

    EXPECT_NE(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    // Set core file name
    string newcorename = corename + "_withDftValue-1D-pos_incst-mask-pos-ext";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string fakefullname = GetPathFromFullName(oldfullname) + "noExistDir" + SEP + \
                      "noOut" + "." + GetSuffix(oldfullname);
    EXPECT_FALSE(rs->outputToFile(fakefullname));
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP + \
                     newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->outputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));
}

INSTANTIATE_TEST_CASE_P(DefaultValue, clsRasterDataTestWithDefaultValue,
                        Values(new inputRasterFiles(asc_file, mask_asc_file),
                               new inputRasterFiles(tif_file, mask_tif_file)));
#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
