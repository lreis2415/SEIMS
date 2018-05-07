/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES           YES            NO               NO
 *        Mask data  :      NO             --            NO               YES
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestMultiPosIncstMaskNoPosExt
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
 *
 */
#include "gtest/gtest.h"
#include <vector>

#include "../../src/data_raster.h"
#include "../../src/utils_filesystem.h"
#ifdef USE_MONGODB
#include "../../src/db_mongoc.h"
#endif

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
using std::vector;
#ifdef USE_MONGODB
using namespace ccgl::db_mongoc;
#endif

namespace {
using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();

string rs1_asc = apppath + "./data/raster/dem_1.asc";
string rs2_asc = apppath + "./data/raster/dem_2.asc";
string rs3_asc = apppath + "./data/raster/dem_3.asc";

string rs1_tif = apppath + "./data/raster/dem_1.tif";
string rs2_tif = apppath + "./data/raster/dem_2.tif";
string rs3_tif = apppath + "./data/raster/dem_3.tif";

string mask_asc_file = apppath + "./data/raster/mask1.asc";
string mask_tif_file = apppath + "./data/raster/mask1.tif";

struct InputRasterFiles {
public:
    InputRasterFiles(const string& rs1, const string& rs2,
                     const string& rs3, const string& maskf) {
        raster_name1 = rs1.c_str();
        raster_name2 = rs2.c_str();
        raster_name3 = rs3.c_str();
        mask_name = maskf.c_str();
    };
    const char* raster_name1;
    const char* raster_name2;
    const char* raster_name3;
    const char* mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestMultiPosIncstMaskNoPosExt: public TestWithParam<InputRasterFiles *> {
public:
    clsRasterDataTestMultiPosIncstMaskNoPosExt() : rs_(nullptr), maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestMultiPosIncstMaskNoPosExt() { delete rs_; }

    void SetUp() OVERRIDE {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs_ = clsRasterData<int>::Init(GetParam()->mask_name, false);
        ASSERT_NE(nullptr, maskrs_);
        // Read raster data with the masked data
        vector<string> filenames;
        filenames.emplace_back(GetParam()->raster_name1);
        filenames.emplace_back(GetParam()->raster_name2);
        filenames.emplace_back(GetParam()->raster_name3);

        rs_ = clsRasterData<float, int>::Init(filenames, true, maskrs_, true);
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
TEST_P(clsRasterDataTestMultiPosIncstMaskNoPosExt, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(73, rs_->GetDataLength()); // m_nCells
    EXPECT_EQ(73, rs_->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ("dem", rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_TRUE(rs_->Is2DRaster());            // m_is2DRaster
    EXPECT_TRUE(rs_->PositionsCalculated());   // m_calcPositions
    EXPECT_FALSE(rs_->PositionsAllocated());   // m_storePositions
    EXPECT_TRUE(rs_->MaskExtented());          // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_EQ(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_NE(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
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
    EXPECT_EQ(3, rs_->GetLayers());
    EXPECT_STREQ("", rs_->GetSrs());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap2D **/
    // layer1 by default
    EXPECT_EQ(64, rs_->GetValidNumber());
    EXPECT_FLOAT_EQ(7.07f, rs_->GetMinimum());
    EXPECT_FLOAT_EQ(10.f, rs_->GetMaximum(1));
    EXPECT_FLOAT_EQ(8.73281250f, rs_->GetAverage(1));
    EXPECT_FLOAT_EQ(0.95102489f, rs_->GetStd());
    EXPECT_FLOAT_EQ(2.93f, rs_->GetRange(1));
    // layer 2
    EXPECT_EQ(60, rs_->GetValidNumber(2));
    EXPECT_FLOAT_EQ(7.07f, rs_->GetMinimum(2));
    EXPECT_FLOAT_EQ(98.49f, rs_->GetMaximum(2));
    EXPECT_FLOAT_EQ(10.23766667f, rs_->GetAverage(2));
    EXPECT_FLOAT_EQ(11.52952953f, rs_->GetStd(2));
    EXPECT_FLOAT_EQ(91.42f, rs_->GetRange(2));

    // layer 3
    EXPECT_EQ(60, rs_->GetValidNumber(3));
    EXPECT_FLOAT_EQ(1.75f, rs_->GetMinimum(3));
    EXPECT_FLOAT_EQ(10.f, rs_->GetMaximum(3));
    EXPECT_FLOAT_EQ(8.43900000f, rs_->GetAverage(3));
    EXPECT_FLOAT_EQ(1.71835454f, rs_->GetStd(3));
    EXPECT_FLOAT_EQ(8.25f, rs_->GetRange(3));

    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_FALSE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_TRUE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(73, ncells);
    EXPECT_EQ(3, nlyrs);
    EXPECT_NE(nullptr, rs_2ddata);
    // raster layer 1
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][0]);
    EXPECT_FLOAT_EQ(7.94f, rs_2ddata[6][0]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][0]);
    EXPECT_FLOAT_EQ(7.62f, rs_2ddata[19][0]);
    // raster layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][1]);
    EXPECT_FLOAT_EQ(7.94f, rs_2ddata[6][1]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][1]);
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[19][1]);
    // raster layer 3
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][2]);
    EXPECT_FLOAT_EQ(7.94f, rs_2ddata[6][2]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][2]);
    EXPECT_FLOAT_EQ(2.62f, rs_2ddata[19][2]);

    // Set core file name
    string newcorename = "dem_2D-pos_incst-mask-nopos-ext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    string newfullname4mongo = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "_mongo." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    MongoClient* conn = MongoClient::Init("127.0.0.1", 27017);
    ASSERT_NE(nullptr, conn);
    string gfsfilename = GetCoreFileName(newfullname);
    MongoGridFs* gfs = new MongoGridFs(conn->GetGridFs("test", "spatial"));
    gfs->RemoveFile(gfsfilename);
    rs_->OutputToMongoDB(gfsfilename, gfs);
    // Currently, the positions data of mask still has not been calculated.
    clsRasterData<float, int>* mongors = clsRasterData<float, int>::Init(gfs, gfsfilename.c_str(), true, maskrs_, true);
    // test mongors data
    EXPECT_EQ(73, mongors->GetCellNumber()); // m_nCells
    EXPECT_EQ(3, mongors->GetLayers());
    EXPECT_EQ(64, mongors->GetValidNumber(1));
    EXPECT_FLOAT_EQ(8.43900000f, mongors->GetAverage(3));
    // output to asc/tif file for comparison
    EXPECT_TRUE(mongors->OutputToFile(newfullname4mongo));
#endif


    /** Copy constructor **/
    clsRasterData<float, int>* copyrs = new clsRasterData<float, int>(rs_);
    // Selected tests
    EXPECT_EQ(73, copyrs->GetCellNumber()); // m_nCells
    EXPECT_EQ(3, copyrs->GetLayers());
    EXPECT_EQ(64, copyrs->GetValidNumber(1));
    EXPECT_FLOAT_EQ(8.43900000f, copyrs->GetAverage(3));

    delete copyrs;
}

#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(MultipleLayers, clsRasterDataTestMultiPosIncstMaskNoPosExt,
    Values(new InputRasterFiles(rs1_asc, rs2_asc, rs3_asc, mask_asc_file),
        new InputRasterFiles(rs1_tif, rs2_tif, rs3_tif, mask_tif_file)));
#else
INSTANTIATE_TEST_CASE_P(MultipleLayers, clsRasterDataTestMultiPosIncstMaskNoPosExt,
                        Values(new InputRasterFiles(rs1_asc, rs2_asc, rs3_asc, mask_asc_file)));
#endif /* USE_GDAL */

} /* namespace */
