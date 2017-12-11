/*!
 * @brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Mask data  :      YES             --            --               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestIncstMaskPos
 * 
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * @cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/02/2017 lj Initial version.
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
string maskcorename = "mask1";
string asc_file = apppath + "../data/" + maskcorename + ".asc";
string tif_file = apppath + "../data/" + maskcorename + ".tif";
const char *asc_file_chars = asc_file.c_str();
const char *tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestIncstMaskPos : public TestWithParam<const char *> {
public:
    clsRasterDataTestIncstMaskPos() : maskrs(nullptr) {}
    virtual ~clsRasterDataTestIncstMaskPos() { delete maskrs; }
    virtual void SetUp() {
        maskrs = clsRasterData<int>::Init(GetParam());  // recommended way
        //maskrs = new clsRasterData<float>(GetParam());  // unsafe way
        ASSERT_NE(nullptr, maskrs);
    }
    virtual void TearDown() {
        delete maskrs;
        maskrs = nullptr;
    }
protected:
    clsRasterData<int> *maskrs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestIncstMaskPos, RasterIO) {
    /// Test members of mask data.
    EXPECT_EQ(73, maskrs->getDataLength());  // m_nCells
    EXPECT_EQ(73, maskrs->getCellNumber());  // m_nCells

    EXPECT_EQ(-9999, maskrs->getNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999, maskrs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(maskcorename, maskrs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(maskrs->Initialized());  // m_initialized
    EXPECT_FALSE(maskrs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(maskrs->PositionsCalculated());  // m_calcPositions
    EXPECT_TRUE(maskrs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(maskrs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(maskrs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(maskrs->validate_raster_data());

    EXPECT_NE(nullptr, maskrs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, maskrs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, maskrs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = maskrs->getRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), maskrs->getLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), maskrs->getCellNumber());

    EXPECT_EQ(9, maskrs->getRows());
    EXPECT_EQ(10, maskrs->getCols());
    EXPECT_FLOAT_EQ(19.f, maskrs->getXllCenter());
    EXPECT_FLOAT_EQ(25.f, maskrs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, maskrs->getCellWidth());
    EXPECT_EQ(1, maskrs->getLayers());
    EXPECT_STREQ("", maskrs->getSRS());
    EXPECT_EQ("", maskrs->getSRSString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(73, maskrs->getValidNumber());
    EXPECT_FLOAT_EQ(1.f, maskrs->getMinimum());
    EXPECT_FLOAT_EQ(1.f, maskrs->getMaximum());
    EXPECT_FLOAT_EQ(1.f, maskrs->getAverage());
    EXPECT_FLOAT_EQ(0.f, maskrs->getSTD());
    EXPECT_FLOAT_EQ(0.f, maskrs->getRange());
    EXPECT_TRUE(maskrs->StatisticsCalculated());

    /** Construct new raster by the mask data and an 1D or 2D array **/
    float *vs = nullptr;
    float **vs2 = nullptr;
    int validmaskcells = maskrs->getCellNumber();
    Initialize1DArray(validmaskcells, vs, 0.f);
    Initialize2DArray(validmaskcells, 3, vs2, 0.f);
    for (int i = 0; i < validmaskcells; i++) {
        vs[i] = i;
        for (int j = 0; j < 3; j++) {
            vs2[i][j] = i * (j + 1);
        }
    }
    /// output array to raster file and destructor the clsRasterData instance immediately
    clsRasterData<float, int> *new1draster = new clsRasterData<float, int>(maskrs, vs);
    string oldfullname = maskrs->getFilePath();
    string new1dfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
        "ctor_mask-pos_array_1d" + "." + GetSuffix(oldfullname);
    new1draster->outputToFile(new1dfullname);

    clsRasterData<float, int> *new2draster = new clsRasterData<float, int>(maskrs, vs2, 3);
    string new2dfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
        "ctor_mask-pos_array_2d" + "." + GetSuffix(oldfullname);
    new2draster->outputToFile(new2dfullname);

    Release1DArray(vs);
    Release2DArray(validmaskcells, vs2);
}
INSTANTIATE_TEST_CASE_P(MaskLayer, clsRasterDataTestIncstMaskPos,
                        Values(asc_file_chars,
                               tif_file_chars));
#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
