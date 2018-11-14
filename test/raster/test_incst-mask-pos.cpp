/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Mask data  :      YES             --            --               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestIncstMaskPos
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

#include "../../src/utils_array.h"
#include "../../src/data_raster.h"
#include "../../src/utils_filesystem.h"

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
using namespace ccgl::utils_array;

namespace {
using testing::TestWithParam;
using testing::Values;

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
class clsRasterDataTestIncstMaskPos: public TestWithParam<const char *> {
public:
    clsRasterDataTestIncstMaskPos() : maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestIncstMaskPos() { delete maskrs_; }

    void SetUp() OVERRIDE {
        maskrs_ = clsRasterData<int>::Init(GetParam()); // recommended way
        //maskrs = new clsRasterData<float>(GetParam());  // unsafe way
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
TEST_P(clsRasterDataTestIncstMaskPos, RasterIO) {
    /// Test members of mask data.
    EXPECT_EQ(73, maskrs_->GetDataLength()); // m_nCells
    EXPECT_EQ(73, maskrs_->GetCellNumber()); // m_nCells

    EXPECT_EQ(-9999, maskrs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999, maskrs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(maskcorename, maskrs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(maskrs_->Initialized());           // m_initialized
    EXPECT_FALSE(maskrs_->Is2DRaster());           // m_is2DRaster
    EXPECT_TRUE(maskrs_->PositionsCalculated());   // m_calcPositions
    EXPECT_TRUE(maskrs_->PositionsAllocated());    // m_storePositions
    EXPECT_FALSE(maskrs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(maskrs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(maskrs_->ValidateRasterData());

    EXPECT_NE(nullptr, maskrs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, maskrs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, maskrs_->GetRasterPositionDataPointer()); // m_rasterPositionData

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

    /** Construct new raster by the mask data and an 1D or 2D array **/
    float* vs = nullptr;
    float** vs2 = nullptr;
    int validmaskcells = maskrs_->GetCellNumber();
    Initialize1DArray(validmaskcells, vs, 0.f);
    Initialize2DArray(validmaskcells, 3, vs2, 0.f);
    for (int i = 0; i < validmaskcells; i++) {
        vs[i] = i;
        for (int j = 0; j < 3; j++) {
            vs2[i][j] = i * (j + 1);
        }
    }
    /// output array to raster file and destructor the clsRasterData instance immediately
    clsRasterData<float, int>* new1draster = new clsRasterData<float, int>(maskrs_, vs);
    string oldfullname = maskrs_->GetFilePath();
    string new1dfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            "ctor_mask-pos_array_1d" + "." + GetSuffix(oldfullname);
    new1draster->OutputToFile(new1dfullname);

    clsRasterData<float, int>* new2draster = new clsRasterData<float, int>(maskrs_, vs2, 3);
    string new2dfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            "ctor_mask-pos_array_2d" + "." + GetSuffix(oldfullname);
    new2draster->OutputToFile(new2dfullname);

    Release1DArray(vs);
    Release2DArray(validmaskcells, vs2);
}
#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(MaskLayer, clsRasterDataTestIncstMaskPos,
    Values(asc_file_chars, tif_file_chars));
#else
INSTANTIATE_TEST_CASE_P(MaskLayer, clsRasterDataTestIncstMaskPos,
                        Values(asc_file_chars));
#endif /* USE_GDAL */

} /* namespace */
