/*!
 * \brief Test description:
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestMaskExceed: Read raster with a mask that exceed raster's extent.
 *
 *            Once a raster used as a mask, its position data must be existed or recalculated.
 *
 *        P.S.1. Copy constructor is also tested here.
 *        P.S.2. MongoDB I/O is also tested if mongo-c-driver configured.
 *
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * \cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * \version 1.3
 * \authors Liangjun Zhu, zlj(at)lreis.ac.cn; crazyzlj(at)gmail.com
 * \remarks 2017-12-02 - lj - Original version.
 *          2018-05-03 - lj - Integrated into CCGL.
 *          2019-11-06 - lj - Allow user specified MongoDB host and port.
 *          2021-07-20 - lj - Update after changes of GetValue and GetValueByIndex.
 *          2021-11-18 - lj - Rewrite unittest cases, avoid redundancy. 
 *
 */
#include "gtest/gtest.h"
#include "../../src/data_raster.hpp"
#include "../../src/utils_filesystem.h"
#include "../../src/utils_time.h"
#include "../../src/utils_string.h"
#include "../../src/utils_array.h"
#ifdef USE_MONGODB
#include "../../src/db_mongoc.h"
#endif
#include "../test_global.h"

using namespace ccgl;
using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
using namespace ccgl::utils_time;
using namespace ccgl::utils_string;
using namespace ccgl::utils_array;
#ifdef USE_MONGODB
using namespace ccgl::db_mongoc;
#endif

extern GlobalEnvironment* GlobalEnv;

namespace {
using ::testing::TestWithParam;
using ::testing::Values;

string Apppath = GetAppPath();
string Corename = "tinydemo_raster_r4c3";
string MaskcorenameS = "tinydemo_mask_r3c3"; // exceed extent of raster data

string AscFile = Apppath + "./data/raster/" + Corename + ".asc";
string TifFile = Apppath + "./data/raster/" + Corename + ".tif";
string MaskAscFile = Apppath + "./data/raster/" + MaskcorenameS + ".asc";
string MaskTifFile = Apppath + "./data/raster/" + MaskcorenameS + ".tif";

struct InputRasterFiles {
public:
    InputRasterFiles(const string& rsf, const string& maskf) {
        raster_name = rsf.c_str();
        mask_name = maskf.c_str();
    }
    const char* raster_name;
    const char* mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestMaskExceed : public TestWithParam<InputRasterFiles*> {
public:
    clsRasterDataTestMaskExceed() : rs_(nullptr), maskrs_(nullptr) {
    }

    virtual ~clsRasterDataTestMaskExceed() { ; }

    void SetUp() OVERRIDE {
        // Read mask data without calculate valid positions
        maskrs_ = IntRaster::Init(GetParam()->mask_name, false);
        ASSERT_NE(nullptr, maskrs_);
        EXPECT_FALSE(maskrs_->PositionsCalculated());
        // Read raster data with mask, do not calculate positions, and do not use mask's extent
        rs_ = FltIntRaster::Init(GetParam()->raster_name, false, maskrs_, false);
        ASSERT_NE(nullptr, rs_);
        EXPECT_TRUE(maskrs_->PositionsCalculated());
#ifdef USE_MONGODB
        client_ = GlobalEnv->client_;
        gfs_ = GlobalEnv->gfs_;
#endif
    }

    void TearDown() OVERRIDE {
        delete rs_;
        delete maskrs_;
    }

protected:
    FltIntRaster* rs_;
    IntRaster* maskrs_;
#ifdef USE_MONGODB
    MongoClient* client_;
    MongoGridFs* gfs_;
#endif
};

// calc_pos = False, use_mask_ext = False
TEST_P(clsRasterDataTestMaskExceed, NoPosNoMaskExt) {
    /// Mask data
    EXPECT_TRUE(maskrs_->PositionsCalculated());
    EXPECT_EQ(5, maskrs_->GetValidNumber());
    EXPECT_EQ(5, maskrs_->GetCellNumber());
    EXPECT_EQ(5, maskrs_->GetDataLength());

    EXPECT_EQ(-9999., maskrs_->GetValueByIndex(-1));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(0));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(1));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(2));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(3));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(4));
    EXPECT_EQ(-9999, maskrs_->GetValueByIndex(5));

    /// Test members after constructing.
    EXPECT_EQ(2, rs_->GetDataLength());
    EXPECT_EQ(2, rs_->GetCellNumber());

    EXPECT_EQ(-9999., rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999., rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(Corename, rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(rs_->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs_->PositionsAllocated());   // m_storePositions
    EXPECT_FALSE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    STRDBL_MAP header_info = rs_->GetRasterHeader();
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_LAYERS)), rs_->GetLayers());
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_CELLSNUM)), rs_->GetCellNumber());

    EXPECT_EQ(1, rs_->GetRows());
    EXPECT_EQ(2, rs_->GetCols());
    EXPECT_DOUBLE_EQ(3., rs_->GetXllCenter());
    EXPECT_DOUBLE_EQ(5., rs_->GetYllCenter());
    EXPECT_DOUBLE_EQ(2., rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(2, rs_->GetValidNumber());
    EXPECT_DOUBLE_EQ(3., rs_->GetMinimum());
    EXPECT_DOUBLE_EQ(4., rs_->GetMaximum());
    EXPECT_DOUBLE_EQ(3.5, rs_->GetAverage());
    EXPECT_DOUBLE_EQ(0.5, rs_->GetStd());
    EXPECT_DOUBLE_EQ(1., rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(2, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(3.f, rs_data[0]);
    EXPECT_FLOAT_EQ(4.f, rs_data[1]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    /// Get cell value by index of raster_data_
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValueByIndex(1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(3));

    /// Get cell value by index of raster_data_/raster_2d_, compatible way
    int tmp_lyr = rs_->GetLayers();
    float* tmp_values = nullptr;
    Initialize1DArray(tmp_lyr, tmp_values, -9999.f);
    rs_->GetValueByIndex(-1, tmp_values); // index exceed, tmp_values will be released
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(0, tmp_values); // nullptr input, tmp_values will be initialized
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValueByIndex(1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);

    /// Get cell value by (row, col)
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValue(0, 0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValue(0, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 1));

    /// Get cell values by (row, col, *&array)
    rs_->GetValue(-1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValue(0, 1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);
    rs_->GetValue(1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    Release1DArray(tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position index, if cal_poc is False, no chance to return -1
    EXPECT_EQ(-2, rs_->GetPosition(-1., -2.));
    EXPECT_EQ(-2, rs_->GetPosition(-1., 9.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 7.));
    EXPECT_EQ(-2, rs_->GetPosition(3.99, 7.05));
    EXPECT_EQ(-2, rs_->GetPosition(5.01, 7.00));
    EXPECT_EQ(-2, rs_->GetPosition(0.01, 5.5));
    EXPECT_EQ(0, rs_->GetPosition(2.01, 5.1));
    EXPECT_EQ(1, rs_->GetPosition(4.5, 5.5));
    EXPECT_EQ(-2, rs_->GetPosition(1., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(3., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(5., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(3., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(5., 1.));

    /** Set value **/
    // Set core file name
    string newcorename = Corename + "_1D-mask-exceed-nopos-nomaskext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    EXPECT_NE(nullptr, client_);
    EXPECT_NE(nullptr, gfs_);
    string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
    string gfsfilename_valid = newcorename + "_valid_" + GetSuffix(oldfullname);
    gfs_->RemoveFile(gfsfilename); // remove if already existed
    gfs_->RemoveFile(gfsfilename_valid);
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename)); // store fullsize raster
    map<string, string> additional_header;
    additional_header[HEADER_MASK_NAME] = rs_->GetCoreName();
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename_valid, additional_header, false));

    // Read from MongoDB without mask layer
    FltRaster* mongors = FltRaster::Init(gfs_, gfsfilename.c_str(), false);

    // Read from MongoDB with predefined valid positions
    FltRaster* mongors_valid = new FltRaster(); // create empty raster, set and read data
    mongors_valid->SetHeader(rs_->GetRasterHeader()); // set header
    int** posdata;
    int poslen;
    rs_->GetRasterPositionData(&poslen, &posdata);
    mongors_valid->SetPositions(poslen, posdata);
    mongors_valid->ReadFromMongoDB(gfs_, gfsfilename_valid);

    // Check the consistency of mongors and mongors_valid
    EXPECT_EQ(mongors->GetCellNumber(), mongors_valid->GetCellNumber());
    EXPECT_EQ(mongors->GetDataLength(), mongors_valid->GetDataLength());
    EXPECT_EQ(mongors->GetValidNumber(), mongors_valid->GetValidNumber());
    EXPECT_DOUBLE_EQ(mongors->GetAverage(), mongors_valid->GetAverage());
    EXPECT_DOUBLE_EQ(mongors->GetStd(), mongors_valid->GetStd());
    EXPECT_EQ(mongors->GetLayers(), mongors_valid->GetLayers());
    EXPECT_EQ(mongors->GetRows(), mongors_valid->GetRows());
    EXPECT_EQ(mongors->GetCols(), mongors_valid->GetCols());
    for (int ir = 0; ir < mongors->GetRows(); ir++) {
        for (int ic = 0; ic < mongors->GetCols(); ic++) {
            EXPECT_FLOAT_EQ(mongors->GetValue(ir, ic), mongors_valid->GetValue(ir, ic));
        }
    }
    delete mongors;
    delete mongors_valid;
#endif
}

// calc_pos = False, use_mask_ext = True
TEST_P(clsRasterDataTestMaskExceed, NoPosUseMaskExt) {
    /// Mask data
    EXPECT_TRUE(maskrs_->PositionsCalculated());
    EXPECT_EQ(5, maskrs_->GetValidNumber());
    EXPECT_EQ(5, maskrs_->GetCellNumber());
    EXPECT_EQ(5, maskrs_->GetDataLength());

    EXPECT_EQ(-9999., maskrs_->GetValueByIndex(-1));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(0));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(1));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(2));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(3));
    EXPECT_EQ(1, maskrs_->GetValueByIndex(4));
    EXPECT_EQ(-9999, maskrs_->GetValueByIndex(5));

    /// Set use_mask_ext
    EXPECT_FALSE(rs_->MaskExtented());
    rs_->SetUseMaskExt();
    EXPECT_TRUE(rs_->MaskExtented());

    /// Test members after constructing.
    EXPECT_EQ(9, rs_->GetDataLength());
    EXPECT_EQ(9, rs_->GetCellNumber());

    EXPECT_EQ(-9999., rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999., rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(Corename, rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(rs_->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs_->PositionsAllocated());   // m_storePositions
    EXPECT_TRUE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    STRDBL_MAP header_info = rs_->GetRasterHeader();
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_LAYERS)), rs_->GetLayers());
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_CELLSNUM)), rs_->GetCellNumber());

    EXPECT_EQ(3, rs_->GetRows());
    EXPECT_EQ(3, rs_->GetCols());
    EXPECT_DOUBLE_EQ(3., rs_->GetXllCenter());
    EXPECT_DOUBLE_EQ(1., rs_->GetYllCenter());
    EXPECT_DOUBLE_EQ(2., rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(2, rs_->GetValidNumber());
    EXPECT_DOUBLE_EQ(3., rs_->GetMinimum());
    EXPECT_DOUBLE_EQ(4., rs_->GetMaximum());
    EXPECT_DOUBLE_EQ(3.5, rs_->GetAverage());
    EXPECT_DOUBLE_EQ(0.5, rs_->GetStd());
    EXPECT_DOUBLE_EQ(1., rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(9, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(3.f, rs_data[0]);
    EXPECT_FLOAT_EQ(4.f, rs_data[1]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[2]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[3]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[4]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[5]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[6]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[7]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[8]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    /// Get cell value by index of raster_data_
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValueByIndex(1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(3));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(4));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(5));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(6));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(7));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(8));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(9));

    /// Get cell value by index of raster_data_/raster_2d_, compatible way
    int tmp_lyr = rs_->GetLayers();
    float* tmp_values = nullptr;
    Initialize1DArray(tmp_lyr, tmp_values, -9999.f);
    rs_->GetValueByIndex(-1, tmp_values); // index exceed, tmp_values will be released
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(0, tmp_values); // nullptr input, tmp_values will be initialized
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValueByIndex(1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);
    rs_->GetValueByIndex(8, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);

    /// Get cell value by (row, col)
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValue(0, 0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValue(0, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(0, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(2, 2));

    /// Get cell values by (row, col, *&array)
    rs_->GetValue(-1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValue(0, 1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);
    rs_->GetValue(1, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);

    Release1DArray(tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position index, if cal_poc is False, no chance to return -1
    EXPECT_EQ(-2, rs_->GetPosition(-1., -2.));
    EXPECT_EQ(-2, rs_->GetPosition(-1., 9.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 7.));
    EXPECT_EQ(-2, rs_->GetPosition(3.99, 7.05));
    EXPECT_EQ(-2, rs_->GetPosition(5.01, 7.00));
    EXPECT_EQ(-2, rs_->GetPosition(0.01, 5.5));
    EXPECT_EQ(0, rs_->GetPosition(2.01, 5.1));
    EXPECT_EQ(1, rs_->GetPosition(4.5, 5.5));
    EXPECT_EQ(2, rs_->GetPosition(6.5, 5.5));
    EXPECT_EQ(3, rs_->GetPosition(3., 3.));
    EXPECT_EQ(4, rs_->GetPosition(5., 3.));
    EXPECT_EQ(5, rs_->GetPosition(7., 3.));
    EXPECT_EQ(6, rs_->GetPosition(3., 1.));
    EXPECT_EQ(7, rs_->GetPosition(5., 1.));
    EXPECT_EQ(8, rs_->GetPosition(7., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 1.));

    /** Set value **/
    // Set core file name
    string newcorename = Corename + "_1D-mask-exceed-nopos-maskext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    EXPECT_NE(nullptr, client_);
    EXPECT_NE(nullptr, gfs_);
    string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
    string gfsfilename_valid = newcorename + "_valid_" + GetSuffix(oldfullname);
    gfs_->RemoveFile(gfsfilename); // remove if already existed
    gfs_->RemoveFile(gfsfilename_valid);
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename)); // store fullsize raster
    map<string, string> additional_header;
    additional_header[HEADER_MASK_NAME] = rs_->GetCoreName();
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename_valid, additional_header, false));

    // Read from MongoDB without mask layer
    FltRaster* mongors = FltRaster::Init(gfs_, gfsfilename.c_str(), false);

    // Read from MongoDB with predefined valid positions
    FltRaster* mongors_valid = new FltRaster(); // create empty raster, set and read data
    mongors_valid->SetHeader(rs_->GetRasterHeader()); // set header
    int** posdata;
    int poslen;
    rs_->GetRasterPositionData(&poslen, &posdata);
    mongors_valid->SetPositions(poslen, posdata);
    mongors_valid->ReadFromMongoDB(gfs_, gfsfilename_valid);

    // Check the consistency of mongors and mongors_valid
    EXPECT_NE(mongors->GetCellNumber(), mongors_valid->GetCellNumber());
    EXPECT_NE(mongors->GetDataLength(), mongors_valid->GetDataLength());
    EXPECT_EQ(mongors->GetValidNumber(), mongors_valid->GetValidNumber());
    EXPECT_DOUBLE_EQ(mongors->GetAverage(), mongors_valid->GetAverage());
    EXPECT_DOUBLE_EQ(mongors->GetStd(), mongors_valid->GetStd());
    EXPECT_EQ(mongors->GetLayers(), mongors_valid->GetLayers());
    EXPECT_EQ(mongors->GetRows(), mongors_valid->GetRows());
    EXPECT_EQ(mongors->GetCols(), mongors_valid->GetCols());
    for (int ir = 0; ir < mongors->GetRows(); ir++) {
        for (int ic = 0; ic < mongors->GetCols(); ic++) {
            EXPECT_FLOAT_EQ(mongors->GetValue(ir, ic), mongors_valid->GetValue(ir, ic));
        }
    }
    delete mongors;
    delete mongors_valid;
#endif
}

// calc_pos = True, use_mask_ext = False
TEST_P(clsRasterDataTestMaskExceed, CalPosNoMaskExt) {
    ASSERT_TRUE(rs_->ValidateRasterData());
    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated
    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData
    EXPECT_FALSE(rs_->PositionsCalculated());
    EXPECT_FALSE(rs_->PositionsAllocated());

    /// Calculate position data
    int valid_count;
    int** valid_positions = nullptr;
    rs_->GetRasterPositionData(&valid_count, &valid_positions);
    EXPECT_TRUE(rs_->PositionsCalculated());
    EXPECT_TRUE(rs_->PositionsAllocated());

    EXPECT_EQ(0, valid_positions[0][0]); // row of the first valid cell
    EXPECT_EQ(0, valid_positions[0][1]); // col of the first valid cell
    EXPECT_EQ(0, valid_positions[1][0]);
    EXPECT_EQ(1, valid_positions[1][1]);

    /// Test members after constructing.
    EXPECT_EQ(2, rs_->GetDataLength());
    EXPECT_EQ(2, rs_->GetCellNumber());

    EXPECT_EQ(-9999., rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999., rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(Corename, rs_->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_TRUE(rs_->PositionsCalculated());  // m_calcPositions
    EXPECT_TRUE(rs_->PositionsAllocated());   // m_storePositions
    EXPECT_FALSE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    STRDBL_MAP header_info = rs_->GetRasterHeader();
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_LAYERS)), rs_->GetLayers());
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_CELLSNUM)), rs_->GetCellNumber());

    EXPECT_EQ(1, rs_->GetRows());
    EXPECT_EQ(2, rs_->GetCols());
    EXPECT_DOUBLE_EQ(3., rs_->GetXllCenter());
    EXPECT_DOUBLE_EQ(5., rs_->GetYllCenter());
    EXPECT_DOUBLE_EQ(2., rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(2, rs_->GetValidNumber());
    EXPECT_DOUBLE_EQ(3., rs_->GetMinimum());
    EXPECT_DOUBLE_EQ(4., rs_->GetMaximum());
    EXPECT_DOUBLE_EQ(3.5, rs_->GetAverage());
    EXPECT_DOUBLE_EQ(0.5, rs_->GetStd());
    EXPECT_DOUBLE_EQ(1., rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(2, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(3.f, rs_data[0]);
    EXPECT_FLOAT_EQ(4.f, rs_data[1]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    /// Get cell value by index of raster_data_
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValueByIndex(1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(3));

    /// Get cell value by index of raster_data_/raster_2d_, compatible way
    int tmp_lyr = rs_->GetLayers();
    float* tmp_values = nullptr;
    Initialize1DArray(tmp_lyr, tmp_values, -9999.f);
    rs_->GetValueByIndex(-1, tmp_values); // index exceed, tmp_values will be released
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(0, tmp_values); // nullptr input, tmp_values will be initialized
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValueByIndex(1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);

    /// Get cell value by (row, col)
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValue(0, 0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValue(0, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 1));

    /// Get cell values by (row, col, *&array)
    rs_->GetValue(-1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValue(0, 1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);
    rs_->GetValue(1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    Release1DArray(tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position index, if cal_poc is False, no chance to return -1
    EXPECT_EQ(-2, rs_->GetPosition(-1., -2.));
    EXPECT_EQ(-2, rs_->GetPosition(-1., 9.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 7.));
    EXPECT_EQ(-2, rs_->GetPosition(3.99, 7.05));
    EXPECT_EQ(-2, rs_->GetPosition(5.01, 7.00));
    EXPECT_EQ(-2, rs_->GetPosition(0.01, 5.5));
    EXPECT_EQ(0, rs_->GetPosition(2.01, 5.1));
    EXPECT_EQ(1, rs_->GetPosition(4.5, 5.5));
    EXPECT_EQ(-2, rs_->GetPosition(1., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(3., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(5., 3.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(3., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(5., 1.));

    /** Set value **/
    // Set core file name
    string newcorename = Corename + "_1D-mask-exceed-calpos-nomaskext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    EXPECT_NE(nullptr, client_);
    EXPECT_NE(nullptr, gfs_);
    string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
    string gfsfilename_valid = newcorename + "_valid_" + GetSuffix(oldfullname);
    gfs_->RemoveFile(gfsfilename); // remove if already existed
    gfs_->RemoveFile(gfsfilename_valid);
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename)); // store fullsize raster
    map<string, string> additional_header;
    additional_header[HEADER_MASK_NAME] = rs_->GetCoreName();
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename_valid, additional_header, false));

    // Read from MongoDB without mask layer
    FltRaster* mongors = FltRaster::Init(gfs_, gfsfilename.c_str(), false);

    // Read from MongoDB with predefined valid positions
    FltRaster* mongors_valid = new FltRaster(); // create empty raster, set and read data
    mongors_valid->SetHeader(rs_->GetRasterHeader()); // set header
    int** posdata;
    int poslen;
    rs_->GetRasterPositionData(&poslen, &posdata);
    mongors_valid->SetPositions(poslen, posdata);
    EXPECT_TRUE(mongors_valid->ReadFromMongoDB(gfs_, gfsfilename_valid));

    // Check the consistency of mongors and mongors_valid
    EXPECT_EQ(mongors->GetCellNumber(), mongors_valid->GetCellNumber());
    EXPECT_EQ(mongors->GetDataLength(), mongors_valid->GetDataLength());
    EXPECT_EQ(mongors->GetValidNumber(), mongors_valid->GetValidNumber());
    EXPECT_DOUBLE_EQ(mongors->GetAverage(), mongors_valid->GetAverage());
    EXPECT_DOUBLE_EQ(mongors->GetStd(), mongors_valid->GetStd());
    EXPECT_EQ(mongors->GetLayers(), mongors_valid->GetLayers());
    EXPECT_EQ(mongors->GetRows(), mongors_valid->GetRows());
    EXPECT_EQ(mongors->GetCols(), mongors_valid->GetCols());
    for (int ir = 0; ir < mongors->GetRows(); ir++) {
        for (int ic = 0; ic < mongors->GetCols(); ic++) {
            EXPECT_FLOAT_EQ(mongors->GetValue(ir, ic), mongors_valid->GetValue(ir, ic));
        }
    }
    delete mongors;
    delete mongors_valid;
#endif
}

// calc_pos = True, use_mask_ext = True
TEST_P(clsRasterDataTestMaskExceed, CalPosUseMaskExt) {
    ASSERT_TRUE(rs_->ValidateRasterData());
    EXPECT_TRUE(rs_->Initialized());           // m_initialized
    EXPECT_FALSE(rs_->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(rs_->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs_->StatisticsCalculated()); // m_statisticsCalculated
    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData
    EXPECT_FALSE(rs_->PositionsCalculated());
    EXPECT_FALSE(rs_->PositionsAllocated());

    /// Calculate position data
    int valid_count;
    int** valid_positions = nullptr;
    rs_->GetRasterPositionData(&valid_count, &valid_positions);
    EXPECT_TRUE(rs_->PositionsCalculated());
    EXPECT_TRUE(rs_->PositionsAllocated());

    EXPECT_EQ(2, valid_count);
    EXPECT_EQ(0, valid_positions[0][0]); // row of the first valid cell
    EXPECT_EQ(0, valid_positions[0][1]); // col of the first valid cell
    EXPECT_EQ(0, valid_positions[1][0]);
    EXPECT_EQ(1, valid_positions[1][1]);

    /// Set to use the extent of mask data
    rs_->SetUseMaskExt();
    EXPECT_FALSE(rs_->PositionsAllocated());
    rs_->GetRasterPositionData(&valid_count, &valid_positions);

    EXPECT_EQ(0, valid_positions[0][0]); // row of the first valid cell
    EXPECT_EQ(0, valid_positions[0][1]); // col of the first valid cell
    EXPECT_EQ(0, valid_positions[1][0]);
    EXPECT_EQ(1, valid_positions[1][1]);
    EXPECT_EQ(1, valid_positions[2][0]);
    EXPECT_EQ(1, valid_positions[2][1]);
    EXPECT_EQ(1, valid_positions[3][0]);
    EXPECT_EQ(2, valid_positions[3][1]);
    EXPECT_EQ(2, valid_positions[4][0]);
    EXPECT_EQ(0, valid_positions[4][1]);

    /// Test members after constructing.
    EXPECT_EQ(5, rs_->GetDataLength());
    EXPECT_EQ(5, rs_->GetCellNumber());

    EXPECT_EQ(-9999., rs_->GetNoDataValue());  // m_noDataValue
    EXPECT_EQ(-9999., rs_->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(Corename, rs_->GetCoreName()); // m_coreFileName

    ASSERT_TRUE(rs_->ValidateRasterData());

    EXPECT_NE(nullptr, rs_->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs_->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, rs_->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    STRDBL_MAP header_info = rs_->GetRasterHeader();
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_LAYERS)), rs_->GetLayers());
    EXPECT_EQ(CVT_INT(header_info.at(HEADER_RS_CELLSNUM)), rs_->GetCellNumber());

    EXPECT_EQ(3, rs_->GetRows());
    EXPECT_EQ(3, rs_->GetCols());
    EXPECT_DOUBLE_EQ(3., rs_->GetXllCenter());
    EXPECT_DOUBLE_EQ(1., rs_->GetYllCenter());
    EXPECT_DOUBLE_EQ(2., rs_->GetCellWidth());
    EXPECT_EQ(1, rs_->GetLayers());
    EXPECT_EQ("", rs_->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(2, rs_->GetValidNumber());
    EXPECT_DOUBLE_EQ(3., rs_->GetMinimum());
    EXPECT_DOUBLE_EQ(4., rs_->GetMaximum());
    EXPECT_DOUBLE_EQ(3.5, rs_->GetAverage());
    EXPECT_DOUBLE_EQ(0.5, rs_->GetStd());
    EXPECT_DOUBLE_EQ(1., rs_->GetRange());
    EXPECT_TRUE(rs_->StatisticsCalculated());

    EXPECT_NE(nullptr, rs_->GetMask()); // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_TRUE(rs_->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(5, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(3.f, rs_data[0]);
    EXPECT_FLOAT_EQ(4.f, rs_data[1]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[2]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[3]);
    EXPECT_FLOAT_EQ(-9999.f, rs_data[4]);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs_->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    /// Get cell value by index of raster_data_
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValueByIndex(1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(2));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(3));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(4));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValueByIndex(5));

    /// Get cell value by index of raster_data_/raster_2d_, compatible way
    int tmp_lyr = rs_->GetLayers();
    float* tmp_values = nullptr;
    Initialize1DArray(tmp_lyr, tmp_values, -9999.f);
    rs_->GetValueByIndex(-1, tmp_values); // index exceed, tmp_values will be released
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValueByIndex(0, tmp_values); // nullptr input, tmp_values will be initialized
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValueByIndex(1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);

    /// Get cell value by (row, col)
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(3.f, rs_->GetValue(0, 0));
    EXPECT_FLOAT_EQ(4.f, rs_->GetValue(0, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs_->GetValue(1, 1));

    /// Get cell values by (row, col, *&array)
    rs_->GetValue(-1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, -1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs_->GetValue(0, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(3.f, tmp_values[0]);
    rs_->GetValue(0, 1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(4.f, tmp_values[0]);
    rs_->GetValue(1, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);

    Release1DArray(tmp_values);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position index, if cal_poc is False, no chance to return -1
    EXPECT_EQ(-2, rs_->GetPosition(-1., -2.));
    EXPECT_EQ(-2, rs_->GetPosition(-1., 9.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 7.));
    EXPECT_EQ(-2, rs_->GetPosition(3.99, 7.05));
    EXPECT_EQ(-2, rs_->GetPosition(5.01, 7.00));
    EXPECT_EQ(-2, rs_->GetPosition(0.01, 5.5));
    EXPECT_EQ(0, rs_->GetPosition(2.01, 5.1));
    EXPECT_EQ(1, rs_->GetPosition(4.5, 5.5));
    EXPECT_EQ(-2, rs_->GetPosition(1., 3.));
    EXPECT_EQ(-1, rs_->GetPosition(3., 3.));
    EXPECT_EQ(2, rs_->GetPosition(5., 3.));
    EXPECT_EQ(3, rs_->GetPosition(7., 3.));
    EXPECT_EQ(4, rs_->GetPosition(3., 1.));
    EXPECT_EQ(-1, rs_->GetPosition(5., 1.));
    EXPECT_EQ(-1, rs_->GetPosition(7., 1.));
    EXPECT_EQ(-2, rs_->GetPosition(1., 1.));
    EXPECT_EQ(-1, rs_->GetPosition(5., 1.));

    /** Set value **/
    // Set core file name
    string newcorename = Corename + "_1D-mask-exceed-calpos-maskext";
    rs_->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs_->GetCoreName());

    /** Output to new file **/
    string oldfullname = rs_->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs_->OutputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    EXPECT_NE(nullptr, client_);
    EXPECT_NE(nullptr, gfs_);
    string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
    string gfsfilename_valid = newcorename + "_valid_" + GetSuffix(oldfullname);
    gfs_->RemoveFile(gfsfilename); // remove if already existed
    gfs_->RemoveFile(gfsfilename_valid);
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename)); // store fullsize raster
    map<string, string> additional_header;
    additional_header[HEADER_MASK_NAME] = rs_->GetCoreName();
    EXPECT_TRUE(rs_->OutputToMongoDB(gfs_, gfsfilename_valid, additional_header, false));

    // Read from MongoDB without mask layer
    FltRaster* mongors = FltRaster::Init(gfs_, gfsfilename.c_str(), false);

    // Read from MongoDB with predefined valid positions
    FltRaster* mongors_valid = new FltRaster(); // create empty raster, set and read data
    mongors_valid->SetHeader(rs_->GetRasterHeader()); // set header
    int** posdata;
    int poslen;
    rs_->GetRasterPositionData(&poslen, &posdata);
    mongors_valid->SetPositions(poslen, posdata);
    mongors_valid->ReadFromMongoDB(gfs_, gfsfilename_valid);

    // Check the consistency of mongors and mongors_valid
    EXPECT_NE(mongors->GetCellNumber(), mongors_valid->GetCellNumber());
    EXPECT_NE(mongors->GetDataLength(), mongors_valid->GetDataLength());
    EXPECT_EQ(mongors->GetValidNumber(), mongors_valid->GetValidNumber());
    EXPECT_DOUBLE_EQ(mongors->GetAverage(), mongors_valid->GetAverage());
    EXPECT_DOUBLE_EQ(mongors->GetStd(), mongors_valid->GetStd());
    EXPECT_EQ(mongors->GetLayers(), mongors_valid->GetLayers());
    EXPECT_EQ(mongors->GetRows(), mongors_valid->GetRows());
    EXPECT_EQ(mongors->GetCols(), mongors_valid->GetCols());
    for (int ir = 0; ir < mongors->GetRows(); ir++) {
        for (int ic = 0; ic < mongors->GetCols(); ic++) {
            EXPECT_FLOAT_EQ(mongors->GetValue(ir, ic), mongors_valid->GetValue(ir, ic));
        }
    }
    delete mongors;
    delete mongors_valid;
#endif
}


#ifdef USE_GDAL
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestMaskExceed,
                        Values(new InputRasterFiles(AscFile, MaskAscFile),
                            new InputRasterFiles(TifFile, MaskTifFile)));
#else
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestMaskExceed,
                        Values(new InputRasterFiles(AscFile, MaskAscFile)));
#endif /* USE_GDAL */

} /* namespace */
