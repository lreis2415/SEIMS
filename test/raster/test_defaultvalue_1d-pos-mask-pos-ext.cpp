/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES           YES            NO               YES
 *        Mask data  :      YES            --            NO               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestWithDefaultValue
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
string corename = "luid";
string maskcorename = "mask1";
string asc_file = apppath + "./data/raster/" + corename + ".asc";
string tif_file = apppath + "./data/raster/" + corename + ".tif";
string mask_asc_file = apppath + "./data/raster/" + maskcorename + ".asc";
string mask_tif_file = apppath + "./data/raster/" + maskcorename + ".tif";

struct InputRasterFiles {
public:
    InputRasterFiles(const string& rsf, const string& maskf) {
        raster_name = rsf.c_str();
        mask_name = maskf.c_str();
    };
    const char* raster_name;
    const char* mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestWithDefaultValue: public TestWithParam<InputRasterFiles *> {
public:
    clsRasterDataTestWithDefaultValue() : rs_(nullptr), maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestWithDefaultValue() { delete rs_; }

    void SetUp() OVERRIDE {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs_ = clsRasterData<int>::Init(GetParam()->mask_name);
        ASSERT_NE(nullptr, maskrs_);
        // Read raster data with the masked data
        rs_ = clsRasterData<int, int>::Init(GetParam()->raster_name, true, maskrs_, true, 4);
        ASSERT_NE(nullptr, rs_);
    }

    void TearDown() OVERRIDE {
        delete rs_;
        delete maskrs_;
        rs_ = nullptr;
        maskrs_ = nullptr;
    }

protected:
    clsRasterData<int, int>* rs_;
    clsRasterData<int>* maskrs_;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestWithDefaultValue, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(73, rs_->GetDataLength()); // m_nCells, which is the same as the extent of mask data
    EXPECT_EQ(73, rs_->GetCellNumber()); // m_nCells

    EXPECT_EQ(-2147483647, rs_->GetNoDataValue()); // m_noDataValue
    EXPECT_EQ(4, rs_->GetDefaultValue());          // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_TRUE(rs_->PositionsCalculated());   // m_calcPositions
    EXPECT_FALSE(rs_->PositionsAllocated());   // m_storePositions
    EXPECT_TRUE(rs_->MaskExtented());          // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    // Set core file name
    string newcorename = corename + "_withDftValue-1D-pos_incst-mask-pos-ext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));
}

#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(DefaultValue, clsRasterDataTestWithDefaultValue,
    Values(new InputRasterFiles(asc_file, mask_asc_file),
        new InputRasterFiles(tif_file, mask_tif_file)));
#else
INSTANTIATE_TEST_CASE_P(DefaultValue, clsRasterDataTestWithDefaultValue,
                        Values(new InputRasterFiles(asc_file, mask_asc_file)));
#endif /* USE_GDAL */

} /* namespace */
