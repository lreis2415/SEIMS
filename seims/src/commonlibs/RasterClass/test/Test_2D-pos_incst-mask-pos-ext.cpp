/*!
 * @brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES           YES            NO               NO
 *        Mask data  :      YES            --            NO               YES
 *
 *        TEST CASE NAME (or TEST SUITE): 
 *            clsRasterDataTestMultiPosIncstMaskPosExt
 *
 *        P.S.1. Copy constructor is also tested here.
 *        P.S.2. MongoDB I/O is also tested if mongo-c-driver configured.
 *
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * @cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/08/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

#include <vector>

using namespace std;

namespace {
#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();

string rs1_asc = apppath + "../data/dem_1.asc";
string rs2_asc = apppath + "../data/dem_2.asc";
string rs3_asc = apppath + "../data/dem_3.asc";

string rs1_tif = apppath + "../data/dem_1.tif";
string rs2_tif = apppath + "../data/dem_2.tif";
string rs3_tif = apppath + "../data/dem_3.tif";

string mask_asc_file = apppath + "../data/mask1.asc";
string mask_tif_file = apppath + "../data/mask1.tif";

struct inputRasterFiles {
public:
    inputRasterFiles(const string &rs1, const string &rs2,
                     const string &rs3, const string &maskf) {
        raster_name1 = rs1.c_str();
        raster_name2 = rs2.c_str();
        raster_name3 = rs3.c_str();
        mask_name = maskf.c_str();
    };
    const char *raster_name1;
    const char *raster_name2;
    const char *raster_name3;
    const char *mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestMultiPosIncstMaskPosExt : public TestWithParam<inputRasterFiles *> {
public:
    clsRasterDataTestMultiPosIncstMaskPosExt() : rs(nullptr), maskrs(nullptr) {}
    virtual ~clsRasterDataTestMultiPosIncstMaskPosExt() { delete rs; }
    virtual void SetUp() {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs = clsRasterData<int>::Init(GetParam()->mask_name, true);
        ASSERT_NE(nullptr, maskrs);
        // Read raster data with the masked data
        vector<string> filenames;
        filenames.emplace_back(GetParam()->raster_name1);
        filenames.emplace_back(GetParam()->raster_name2);
        filenames.emplace_back(GetParam()->raster_name3);

        rs = clsRasterData<float, int>::Init(filenames, true, maskrs, true);
        ASSERT_NE(nullptr, rs);
    }
    virtual void TearDown() {
        delete rs;
        delete maskrs;
        rs = nullptr;
        maskrs = nullptr;
    }
protected:
    clsRasterData<float, int> *rs;
    clsRasterData<int> *maskrs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestMultiPosIncstMaskPosExt, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(73, rs->getDataLength());  // m_nCells
    EXPECT_EQ(73, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ("dem", rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_TRUE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_TRUE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(rs->validate_raster_data());

    EXPECT_EQ(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_NE(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs->getRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs->getLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs->getCellNumber());

    EXPECT_EQ(9, rs->getRows());
    EXPECT_EQ(10, rs->getCols());
    EXPECT_FLOAT_EQ(19.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(25.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->getCellWidth());
    EXPECT_EQ(3, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap2D **/
    // layer1 by default
    EXPECT_EQ(64, rs->getValidNumber());
    EXPECT_FLOAT_EQ(7.07f, rs->getMinimum());
    EXPECT_FLOAT_EQ(10.f, rs->getMaximum(1));
    EXPECT_FLOAT_EQ(8.73281250f, rs->getAverage(1));
    EXPECT_FLOAT_EQ(0.95102489f, rs->getSTD());
    EXPECT_FLOAT_EQ(2.93f, rs->getRange(1));
    // layer 2
    EXPECT_EQ(60, rs->getValidNumber(2));
    EXPECT_FLOAT_EQ(7.07f, rs->getMinimum(2));
    EXPECT_FLOAT_EQ(98.49f, rs->getMaximum(2));
    EXPECT_FLOAT_EQ(10.23766667f, rs->getAverage(2));
    EXPECT_FLOAT_EQ(11.52952953f, rs->getSTD(2));
    EXPECT_FLOAT_EQ(91.42f, rs->getRange(2));

    // layer 3
    EXPECT_EQ(60, rs->getValidNumber(3));
    EXPECT_FLOAT_EQ(1.75f, rs->getMinimum(3));
    EXPECT_FLOAT_EQ(10.f, rs->getMaximum(3));
    EXPECT_FLOAT_EQ(8.43900000f, rs->getAverage(3));
    EXPECT_FLOAT_EQ(1.71835454f, rs->getSTD(3));
    EXPECT_FLOAT_EQ(8.25f, rs->getRange(3));

    EXPECT_TRUE(rs->StatisticsCalculated());

    EXPECT_NE(nullptr, rs->getMask());  // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float *rs_data = nullptr;
    EXPECT_FALSE(rs->getRasterData(&ncells, &rs_data));  // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float **rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_TRUE(rs->get2DRasterData(&ncells, &nlyrs, &rs_2ddata));  // m_raster2DData
    EXPECT_EQ(73, ncells);
    EXPECT_EQ(3, nlyrs);
    EXPECT_NE(nullptr, rs_2ddata);
    // raster layer 1
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][0]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][0]);
    EXPECT_FLOAT_EQ(8.89f, rs_2ddata[27][0]);
    // raster layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][1]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][1]);
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[27][1]);
    // raster layer 3
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][2]);
    EXPECT_FLOAT_EQ(9.85f, rs_2ddata[72][2]);
    EXPECT_FLOAT_EQ(8.89f, rs_2ddata[27][2]);

    // Set core file name
    string newcorename = "dem_2D-pos_incst-mask-pos-ext";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string fakefullname = GetPathFromFullName(oldfullname) + "noExistDir" + SEP +
        "noOut" + "." + GetSuffix(oldfullname);
    EXPECT_FALSE(rs->outputToFile(fakefullname));
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
        newcorename + "." + GetSuffix(oldfullname);
    string newfullname4mongo = GetPathFromFullName(oldfullname) + "result" + SEP +
        newcorename + "_mongo." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->outputToFile(newfullname));

    /** Copy constructor **/
    clsRasterData<float, int> *copyrs = new clsRasterData<float, int>(rs);
    // Selected tests
    EXPECT_EQ(73, copyrs->getCellNumber());  // m_nCells
    EXPECT_EQ(3, copyrs->getLayers());
    EXPECT_EQ(64, copyrs->getValidNumber(1));
    EXPECT_FLOAT_EQ(8.43900000f, copyrs->getAverage(3));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    MongoClient *conn = MongoClient::Init("127.0.0.1", 27017);
    ASSERT_NE(nullptr, conn);
    string gfsfilename = newfullname + "_" + GetSuffix(oldfullname);
    MongoGridFS *gfs = new MongoGridFS(conn->getGridFS("test", "spatial"));
    gfs->removeFile(gfsfilename);
    copyrs->outputToMongoDB(gfsfilename, gfs);
    double stime = TimeCounting();
    clsRasterData<float, int> *mongors = clsRasterData<float, int>::Init(gfs, gfsfilename.c_str(), true, maskrs, true);
    cout << "Reading parameter finished, TIMESPAN " << ValueToString(TimeCounting() - stime) << " sec." << endl;
    // test mongors data
    EXPECT_EQ(73, mongors->getCellNumber());  // m_nCells
    EXPECT_EQ(3, mongors->getLayers());
    EXPECT_EQ(64, mongors->getValidNumber(1));
    EXPECT_FLOAT_EQ(8.43900000f, mongors->getAverage(3));
    // output to asc/tif file for comparison
    EXPECT_TRUE(mongors->outputToFile(newfullname4mongo));
#endif
    delete copyrs;
}

INSTANTIATE_TEST_CASE_P(MultipleLayers, clsRasterDataTestMultiPosIncstMaskPosExt,
                        Values(new inputRasterFiles(rs1_asc, rs2_asc, rs3_asc, mask_asc_file),
                               new inputRasterFiles(rs1_tif, rs2_tif, rs3_tif, mask_tif_file)));
#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
