/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Mask data  :      YES             --            --               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestIncstMaskNoPos
 *
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * \cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * \version 1.1
 * \authors Liangjun Zhu (zlj@lreis.ac.cn)
 * \revised 2017-12-02 - lj - Original version.
 *          2018-05-03 - lj - Integrated into CCGL.
 *
 */
#include "gtest/gtest.h"

#include "../../src/data_raster.h"
#include "../../src/utils_filesystem.h"

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;

namespace {
using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string maskcorename = "mask1";
string asc_file = apppath + "./data/raster/" + maskcorename + ".asc";
string tif_file = apppath + "./data/raster/" + maskcorename + ".tif";
const char* asc_file_chars = asc_file.c_str();
const char* tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestIncstMaskNoPos: public TestWithParam<const char *> {
public:
    clsRasterDataTestIncstMaskNoPos() : maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestIncstMaskNoPos() { delete maskrs_; }

    void SetUp() OVERRIDE {
        maskrs_ = clsRasterData<int>::Init(GetParam(), false); // recommended way
        //maskrs = new clsRasterData<float>(GetParam(), false);  // unsafe way
        ASSERT_NE(nullptr, maskrs_);
    }

    void TearDown() OVERRIDE {
        delete maskrs_;
        maskrs_ = nullptr;
    }

protected:
    clsRasterData<int>* maskrs_;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestIncstMaskNoPos, RasterIO) {
    /// Test members of mask data.
    EXPECT_EQ(90, maskrs_->GetDataLength()); // m_nCells
    EXPECT_EQ(90, maskrs_->GetCellNumber()); // m_nCells

    EXPECT_EQ(-9999, maskrs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999, maskrs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(maskcorename, maskrs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(maskrs_->Initialized());           // m_initialized
    EXPECT_FALSE(maskrs_->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(maskrs_->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(maskrs_->PositionsAllocated());   // m_storePositions
    EXPECT_FALSE(maskrs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(maskrs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(maskrs_->ValidateRasterData());

    EXPECT_NE(nullptr, maskrs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, maskrs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, maskrs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = maskrs_->GetRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), maskrs_->GetLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), maskrs_->GetCellNumber());

    EXPECT_EQ(9, maskrs_->GetRows());
    EXPECT_EQ(10, maskrs_->GetCols());
    EXPECT_FLOAT_EQ(19.f, maskrs_->GetXllCenter());
    EXPECT_FLOAT_EQ(25.f, maskrs_->GetYllCenter());
    EXPECT_FLOAT_EQ(2.f, maskrs_->GetCellWidth());
    EXPECT_EQ(1, maskrs_->GetLayers());
    EXPECT_EQ("", maskrs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(73, maskrs_->GetValidNumber());
    EXPECT_FLOAT_EQ(1.f, maskrs_->GetMinimum());
    EXPECT_FLOAT_EQ(1.f, maskrs_->GetMaximum());
    EXPECT_FLOAT_EQ(1.f, maskrs_->GetAverage());
    EXPECT_FLOAT_EQ(0.f, maskrs_->GetStd());
    EXPECT_FLOAT_EQ(0.f, maskrs_->GetRange());
    EXPECT_TRUE(maskrs_->StatisticsCalculated());
}

#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(MaskLayer, clsRasterDataTestIncstMaskNoPos,
    Values(asc_file_chars, tif_file_chars));
#else
INSTANTIATE_TEST_CASE_P(MaskLayer, clsRasterDataTestIncstMaskNoPos,
                        Values(asc_file_chars));
#endif /* USE_GDAL */

} /* namespace */
