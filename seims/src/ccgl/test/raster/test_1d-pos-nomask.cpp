/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES            --            --               YES
 *        Mask data  :      --             --            --               --
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestPosNoMask
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
#ifdef USE_MONGODB
#include "../../src/db_mongoc.h"
#endif

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
#ifdef USE_MONGODB
using namespace ccgl::db_mongoc;
#endif

namespace {
using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string corename = "dem_2";
string asc_file = apppath + "./data/raster/" + corename + ".asc";
string tif_file = apppath + "./data/raster/" + corename + ".tif";
const char* asc_file_chars = asc_file.c_str();
const char* tif_file_chars = tif_file.c_str();

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is file name (const char*) which we call in fixture's SetUp()
//to create and store an instance of clsRasterData<float>*.
class clsRasterDataTestPosNoMask: public TestWithParam<const char *> {
public:
    clsRasterDataTestPosNoMask() : rs_(nullptr) {
    }

    virtual ~clsRasterDataTestPosNoMask() { delete rs_; }

    void SetUp() override {
        rs_ = clsRasterData<float>::Init(GetParam()); // recommended way
        //rs = new clsRasterData<float>(GetParam());  // unsafe way
        ASSERT_NE(nullptr, rs_);
    }

    void TearDown() override {
        delete rs_;
        rs_ = nullptr;
    }

protected:
    clsRasterData<float>* rs_;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestPosNoMask, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(541, rs_->GetDataLength()); // m_nCells
    EXPECT_EQ(541, rs_->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_TRUE(rs_->PositionsCalculated());   // m_calcPositions
    EXPECT_TRUE(rs_->PositionsAllocated());    // m_storePositions
    EXPECT_FALSE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs_->GetRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs_->GetLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs_->GetCellNumber());

    EXPECT_EQ(20, rs_->GetRows());
    EXPECT_EQ(30, rs_->GetCols());
    EXPECT_FLOAT_EQ(1.f, rs_->GetXllCenter());
    EXPECT_FLOAT_EQ(1.f, rs_->GetYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_STREQ("", rs_->GetSrs());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(541, rs_->GetValidNumber());
    EXPECT_FLOAT_EQ(2.75f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(98.49f, rs_->GetMaximum());
    EXPECT_FLOAT_EQ(9.20512f, rs_->GetAverage());
    EXPECT_FLOAT_EQ(5.612893f, rs_->GetStd());
    EXPECT_FLOAT_EQ(95.74f, rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs_->GetMask()); // m_mask

    /** Test getting position data **/
    int ncells = -1;
    int** positions = nullptr;
    rs_->GetRasterPositionData(&ncells, &positions); // m_rasterPositionData
    EXPECT_EQ(541, ncells);
    EXPECT_NE(nullptr, positions);
    // index = 0, row = 0 and col = 1
    EXPECT_EQ(0, positions[0][0]);
    EXPECT_EQ(1, positions[0][1]);
    // index = 540, row = 19 and col = 29
    EXPECT_EQ(19, positions[540][0]);
    EXPECT_EQ(29, positions[540][1]);


    /** Test getting raster data **/
    ncells = -1;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(541, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(9.9f, rs_data[0]);
    EXPECT_FLOAT_EQ(7.21f, rs_data[540]);
    EXPECT_FLOAT_EQ(9.43f, rs_data[29]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(9.9f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(7.21f, rs_->GetValueByIndex(540, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(541, 1));
    EXPECT_FLOAT_EQ(9.43f, rs_->GetValueByIndex(29));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(541, 2));

    int tmp_lyr;
    float* tmp_values;
    rs_->GetValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(20, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.06f, rs_->GetValue(2, 4));
    EXPECT_FLOAT_EQ(8.06f, rs_->GetValue(2, 4, 1));

    rs_->GetValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    rs_->GetValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);

    // Get position
    EXPECT_EQ(29, rs_->GetPosition(4.05f, 37.95f));
    EXPECT_EQ(29, rs_->GetPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string newcorename = corename + "_1D-pos-nomask";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    // Set raster data value
    rs_->SetValue(2, 4, 0.806f);
    EXPECT_FLOAT_EQ(0.806f, rs_->GetValue(2, 4));
    rs_->SetValue(0, 0, 1.f);
    EXPECT_NE(1.f, rs_->GetValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, 0));

    // update statistics
    rs_->UpdateStatistics(); // Should be mannualy invoked in your project!
    EXPECT_FLOAT_EQ(0.806f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(9.19171165f, rs_->GetAverage());
    EXPECT_FLOAT_EQ(5.62426552f, rs_->GetStd());
    EXPECT_FLOAT_EQ(97.684f, rs_->GetRange());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    string newfullname4mongo = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "_mongo." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    MongoClient* conn = MongoClient::Init("127.0.0.1", 27017);
    ASSERT_NE(nullptr, conn);
    string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
    MongoGridFs* gfs = new MongoGridFs(conn->GetGridFs("test", "spatial"));
    gfs->RemoveFile(gfsfilename);
    rs_->OutputToMongoDB(gfsfilename, gfs);
    clsRasterData<float>* mongors = clsRasterData<float>::Init(gfs, gfsfilename.c_str());
    // test mongors data
    EXPECT_EQ(541, mongors->GetCellNumber()); // m_nCells
    EXPECT_EQ(1, mongors->GetLayers());
    EXPECT_EQ(541, mongors->GetValidNumber());
    EXPECT_FLOAT_EQ(0.806f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(9.19171165f, rs_->GetAverage());
    EXPECT_FLOAT_EQ(5.62426552f, rs_->GetStd());
    EXPECT_FLOAT_EQ(97.684f, rs_->GetRange());
    // output to asc/tif file for comparison
    EXPECT_TRUE(mongors->OutputToFile(newfullname4mongo));
    EXPECT_TRUE(FileExists(newfullname4mongo));
#endif
}
#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestPosNoMask,
    Values(asc_file_chars, tif_file_chars));
#else
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestPosNoMask,
                        Values(asc_file_chars));
#endif /* USE_GDAL */

} /* namespace */
