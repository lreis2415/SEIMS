/*!
 * @brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      NO             --            --               YES
 *        Mask data  :      --             --            --               --
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestNoPosNoMask
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
string corename = "dem_2";
string asc_file = apppath + "../data/" + corename + ".asc";
string tif_file = apppath + "../data/" + corename + ".tif";
const char *asc_file_chars = asc_file.c_str();
const char *tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestNoPosNoMask : public TestWithParam<const char *> {
public:
    clsRasterDataTestNoPosNoMask() : rs(nullptr) {}
    virtual ~clsRasterDataTestNoPosNoMask() { delete rs; }
    virtual void SetUp() {
        rs = clsRasterData<float>::Init(GetParam(), false);  // recommended way
        //rs = new clsRasterData<float>(GetParam(), false);  // unsafe way
        ASSERT_NE(nullptr, rs);
    }
    virtual void TearDown() {
        delete rs;
        rs = nullptr;
    }
protected:
    clsRasterData<float> *rs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestNoPosNoMask, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(600, rs->getDataLength());  // m_nCells, which will be nRows * nCols
    EXPECT_EQ(600, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_FALSE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_FALSE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(rs->validate_raster_data());

    EXPECT_NE(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_EQ(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs->getRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs->getLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs->getCellNumber());

    EXPECT_EQ(20, rs->getRows());
    EXPECT_EQ(30, rs->getCols());
    EXPECT_FLOAT_EQ(1.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(1.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->getCellWidth());
    EXPECT_EQ(1, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(541, rs->getValidNumber());
    EXPECT_FLOAT_EQ(2.75f, rs->getMinimum());
    EXPECT_FLOAT_EQ(98.49f, rs->getMaximum());
    EXPECT_FLOAT_EQ(9.20512f, rs->getAverage());
    EXPECT_FLOAT_EQ(5.612893f, rs->getSTD());
    EXPECT_FLOAT_EQ(95.74f, rs->getRange());
    EXPECT_TRUE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->getMask());  // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float *rs_data = nullptr;
    EXPECT_TRUE(rs->getRasterData(&ncells, &rs_data));  // m_rasterData
    EXPECT_EQ(600, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[0]);
    EXPECT_FLOAT_EQ(7.21f, rs_data[599]);
    EXPECT_FLOAT_EQ(8.6f, rs_data[540]);

    float **rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs->get2DRasterData(&ncells, &nlyrs, &rs_2ddata));  // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(0));
    EXPECT_FLOAT_EQ(7.21f, rs->getValueByIndex(599, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(600, 1));
    EXPECT_FLOAT_EQ(9.22f, rs->getValueByIndex(29));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(600, 2));

    int tmp_lyr;
    float *tmp_values;
    rs->getValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValueByIndex(1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(20, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4, 1));

    rs->getValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    rs->getValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    // Get position
    EXPECT_EQ(32, rs->getPosition(4.05f, 37.95f));
    EXPECT_EQ(32, rs->getPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string newcorename = corename + "_1D-nopos-nomask";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    // Set raster data value
    rs->setValue(2, 4, 0.806f);
    EXPECT_FLOAT_EQ(0.806f, rs->getValue(2, 4));
    rs->setValue(0, 0, 1.f);
    EXPECT_EQ(1.f, rs->getValue(0, 0));

    // update statistics
    rs->updateStatistics();  // Should be manually invoked in your project!
    EXPECT_FLOAT_EQ(0.806f, rs->getMinimum());
    EXPECT_FLOAT_EQ(9.17659779f, rs->getAverage());
    EXPECT_FLOAT_EQ(5.63006041f, rs->getSTD());
    EXPECT_FLOAT_EQ(97.684f, rs->getRange());

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string fakefullname = GetPathFromFullName(oldfullname) + "noExistDir" + SEP +
        "noOut" + "." + GetSuffix(oldfullname);
    EXPECT_FALSE(rs->outputToFile(fakefullname));
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
        newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->outputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

    /* Get position data, which will be calculated if not existed */
    ncells = -1;
    int **positions = nullptr;
    rs->getRasterPositionData(&ncells, &positions);  // m_rasterPositionData
    EXPECT_TRUE(rs->PositionsCalculated());
    EXPECT_TRUE(rs->PositionsAllocated());
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());
    EXPECT_EQ(542, ncells);
    EXPECT_NE(nullptr, positions);
    // index = 0, row = 0 and col = 0
    EXPECT_EQ(0, positions[0][0]);
    EXPECT_EQ(0, positions[0][1]);
    // index = 540, row = 19 and col = 29
    EXPECT_EQ(19, positions[540][0]);
    EXPECT_EQ(29, positions[541][1]);

    // In the meantime, other related variables are updated, test some of them.
    EXPECT_EQ(542, rs->getCellNumber());  // m_nCells
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(1.f, rs->getValueByIndex(0));
    EXPECT_FLOAT_EQ(7.21f, rs->getValueByIndex(541, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(542, 1));
    EXPECT_FLOAT_EQ(9.43f, rs->getValueByIndex(30));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(541, 2));
}

// In order to run value-parameterized tests, you need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// You can instantiate them in a different translation module, or even
// instantiate them several times.
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestNoPosNoMask,
                        Values(asc_file_chars,
                               tif_file_chars));
#else
//Google Test may not support value-parameterized tests with some
//compilers. If we use conditional compilation to compile out all
//code referring to the gtest_main library, MSVC linker will not link
//that library at all and consequently complain about missing entry
//point defined in that library (fatal error LNK1561: entry point
//must be defined). This dummy test keeps gtest_main linked in.
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
