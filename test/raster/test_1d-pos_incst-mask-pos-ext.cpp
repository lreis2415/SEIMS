/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES           YES            NO               YES
 *        Mask data  :      YES            --            NO               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestPosIncstMaskPosExt
 *
 *        P.S.1. Copy constructor is also tested here.
 *        P.S.2. MongoDB I/O is also tested if mongo-c-driver configured.
 *
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * \cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * \version 1.1
 * \authors Liangjun Zhu (zlj@lreis.ac.cn)
 * \revised 2017-12-02 - lj - Original version.
 *          2018-05-03 - lj - Integrated into CCGL.
 *          2019-11-06 - lj - Allow user specified MongoDB host and port.
 *
 */
#include "gtest/gtest.h"
#include "../../src/data_raster.h"
#include "../../src/utils_filesystem.h"
#include "../../src/utils_time.h"
#include "../../src/utils_string.h"
#ifdef USE_MONGODB
#include "../../src/db_mongoc.h"
#endif
#include "../test_global.h"

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
using namespace ccgl::utils_time;
using namespace ccgl::utils_string;
#ifdef USE_MONGODB
using namespace ccgl::db_mongoc;
#endif

extern GlobalEnvironment* GlobalEnv;

namespace {
using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string corename = "dem_2";
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
class clsRasterDataTestPosIncstMaskPosExt: public TestWithParam<InputRasterFiles *> {
public:
    clsRasterDataTestPosIncstMaskPosExt() : rs_(nullptr), maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestPosIncstMaskPosExt() { delete rs_; }

    void SetUp() OVERRIDE {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs_ = clsRasterData<int>::Init(GetParam()->mask_name);
        ASSERT_NE(nullptr, maskrs_);
        // Read raster data with the masked data
        rs_ = clsRasterData<float, int>::Init(GetParam()->raster_name, true, maskrs_);
        ASSERT_NE(nullptr, rs_);
    }

    void TearDown() OVERRIDE {
        delete rs_;
        delete maskrs_;
        rs_ = nullptr;
        maskrs_ = nullptr;
    }

protected:
    clsRasterData<float, int>* rs_;
    clsRasterData<int>* maskrs_;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestPosIncstMaskPosExt, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(73, rs_->GetDataLength()); // m_nCells, which is the same as the extent of mask data
    EXPECT_EQ(73, rs_->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetDefaultValue()); // m_defaultValue

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

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs_->GetRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs_->GetLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs_->GetCellNumber());

    EXPECT_EQ(9, rs_->GetRows());
    EXPECT_EQ(10, rs_->GetCols());
    EXPECT_FLOAT_EQ(19.f, rs_->GetXllCenter());
    EXPECT_FLOAT_EQ(25.f, rs_->GetYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(60, rs_->GetValidNumber());
    EXPECT_FLOAT_EQ(7.07f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(98.49f, rs_->GetMaximum());
    EXPECT_FLOAT_EQ(10.23766667f, rs_->GetAverage());
    EXPECT_FLOAT_EQ(11.52952953f, rs_->GetStd());
    EXPECT_FLOAT_EQ(91.42f, rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(73, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[0]);
    EXPECT_FLOAT_EQ(7.94f, rs_data[6]);
    EXPECT_FLOAT_EQ(9.85f, rs_data[72]);
    EXPECT_FLOAT_EQ(8.77f, rs_data[16]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(7.94f, rs_->GetValueByIndex(6));
    EXPECT_FLOAT_EQ(9.85f, rs_->GetValueByIndex(72, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(73, 1));
    EXPECT_FLOAT_EQ(8.77f, rs_->GetValueByIndex(16));
    EXPECT_FLOAT_EQ(9.33f, rs_->GetValueByIndex(18));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(90, 2));

    int tmp_lyr;
    float* tmp_values;
    rs_->GetValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);

    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(9, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, 10));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.77f, rs_->GetValue(2, 4));
    EXPECT_FLOAT_EQ(8.77f, rs_->GetValue(2, 4, 1));

    rs_->GetValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.94f, tmp_values[0]);
    rs_->GetValue(1, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.62f, tmp_values[0]);

    // Get position
    EXPECT_EQ(14, rs_->GetPosition(22.05f, 37.95f)); // row 2, col 2
    EXPECT_EQ(14, rs_->GetPosition(23.95f, 36.05f));

    /** Set value **/

    // Set raster data value
    rs_->SetValue(2, 4, 0.877f);
    EXPECT_FLOAT_EQ(0.877f, rs_->GetValue(2, 4));
    rs_->SetValue(0, 2, 1.f);
    EXPECT_NE(1.f, rs_->GetValue(0, 2));

    // update statistics
    rs_->UpdateStatistics(); // Should be manually invoked in your project!
    EXPECT_FLOAT_EQ(0.877f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(10.10611667f, rs_->GetAverage());
    EXPECT_FLOAT_EQ(11.59039314f, rs_->GetStd());
    EXPECT_FLOAT_EQ(97.613f, rs_->GetRange());

    // Set core file name
    string newcorename = corename + "_1D-pos_incst-mask-pos-ext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    string newfullname4mongo = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "_mongo." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

    /** Copy constructor **/
    clsRasterData<float, int>* copyrs = new clsRasterData<float, int>(rs_);

    clsRasterData<float, int> copyrs2(rs_);

    clsRasterData<float, int> copyrs3;
    copyrs3.Copy(rs_);

    // Selected tests
    EXPECT_EQ(73, copyrs->GetCellNumber()); // m_nCells
    EXPECT_EQ(1, copyrs->GetLayers());
    EXPECT_EQ(60, copyrs->GetValidNumber());
    EXPECT_FLOAT_EQ(10.10611667f, copyrs->GetAverage());

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    MongoClient* conn = MongoClient::Init(GlobalEnv->mongoHost.c_str(), GlobalEnv->mongoPort);
    if (nullptr != conn) {
        string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
        MongoGridFs* gfs = new MongoGridFs(conn->GetGridFs("test", "spatial"));
        gfs->RemoveFile(gfsfilename);
        copyrs->OutputToMongoDB(gfsfilename, gfs);
        double stime = TimeCounting();
        clsRasterData<float, int>* mongors = clsRasterData<float, int>::Init(gfs, gfsfilename.c_str(), true, maskrs_,
                                                                             true);
        cout << "Reading parameter finished, TIMESPAN " << ValueToString(TimeCounting() - stime) << " sec." << endl;
        // test mongors data
        EXPECT_EQ(73, mongors->GetCellNumber()); // m_nCells
        EXPECT_EQ(1, mongors->GetLayers());
        EXPECT_EQ(60, mongors->GetValidNumber());
        EXPECT_FLOAT_EQ(10.10611667f, mongors->GetAverage());
        // output to asc/tif file for comparison
        EXPECT_TRUE(rs_->OutputToFile(newfullname4mongo));
        EXPECT_TRUE(FileExists(newfullname4mongo));
    }
#endif
    delete copyrs;
}

#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestPosIncstMaskPosExt,
    Values(new InputRasterFiles(asc_file, mask_asc_file),
        new InputRasterFiles(tif_file, mask_tif_file)));
#else
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestPosIncstMaskPosExt,
                        Values(new InputRasterFiles(asc_file, mask_asc_file)));
#endif /* USE_GDAL */

} /* namespace */
