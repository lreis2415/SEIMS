/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      YES            --            --               NO
 *        Mask data  :      --             --            --               --
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestMultiPosNoMask
 *
 *        Since the core code is irrelevant with the format of raster data, we take tiff
 *        as example here.
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

TEST(clsRasterDataTestMultiPosNoMask, RasterIO) {
    /// 0. Read multiple raster data.
    string apppath = GetAppPath();
    vector<string> filenames;
#ifdef USE_GDAL
    string suffix = ".tif";
#else
    string suffix = ".asc";
#endif /* USE_GDAL */
    filenames.emplace_back(apppath + "./data/raster/dem_1" + suffix);
    filenames.emplace_back(apppath + "./data/raster/dem_2" + suffix);
    filenames.emplace_back(apppath + "./data/raster/dem_3" + suffix);
    clsRasterData<float>* rs = clsRasterData<float>::Init(filenames); // recommended way
    //clsRasterData<float> *rs = new clsRasterData<float>(filenames);  // unsafe way

    ASSERT_NE(nullptr, rs);

    /// 1. Test members after constructing.
    EXPECT_EQ(545, rs->GetDataLength()); // m_nCells
    EXPECT_EQ(545, rs->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ("dem", rs->GetCoreName());                           // m_coreFileName
    EXPECT_EQ("dem_%d", ccgl::GetCoreFileName(rs->GetFilePath())); // m_filePathName

    EXPECT_TRUE(rs->Initialized());           // m_initialized
    EXPECT_TRUE(rs->Is2DRaster());            // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());   // m_calcPositions
    EXPECT_TRUE(rs->PositionsAllocated());    // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs->ValidateRasterData());

    EXPECT_EQ(nullptr, rs->GetRasterDataPointer());         // m_rasterData
    EXPECT_NE(nullptr, rs->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_NE(nullptr, rs->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs->GetRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs->GetLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs->GetCellNumber());

    EXPECT_EQ(20, rs->GetRows());
    EXPECT_EQ(30, rs->GetCols());
    EXPECT_FLOAT_EQ(1.f, rs->GetXllCenter());
    EXPECT_FLOAT_EQ(1.f, rs->GetYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->GetCellWidth());
    EXPECT_EQ(3, rs->GetLayers());
    EXPECT_STREQ("", rs->GetSrs());
    EXPECT_EQ("", rs->GetSrsString());

    /** Calc and get basic statistics, m_statsMap2D **/
    // layer 1
    EXPECT_EQ(545, rs->GetValidNumber(1));
    EXPECT_FLOAT_EQ(7.07f, rs->GetMinimum(1));
    EXPECT_FLOAT_EQ(10.f, rs->GetMaximum(1));
    EXPECT_FLOAT_EQ(8.693963f, rs->GetAverage(1));
    EXPECT_FLOAT_EQ(0.870768f, rs->GetStd(1));
    EXPECT_FLOAT_EQ(2.93f, rs->GetRange(1));
    // layer 2
    EXPECT_EQ(541, rs->GetValidNumber(2));
    EXPECT_FLOAT_EQ(2.75f, rs->GetMinimum(2));
    EXPECT_FLOAT_EQ(98.49f, rs->GetMaximum(2));
    EXPECT_FLOAT_EQ(9.20512f, rs->GetAverage(2));
    EXPECT_FLOAT_EQ(5.612893f, rs->GetStd(2));
    EXPECT_FLOAT_EQ(95.74f, rs->GetRange(2));
    // layer 3
    EXPECT_EQ(540, rs->GetValidNumber(3));
    EXPECT_FLOAT_EQ(0.6f, rs->GetMinimum(3));
    EXPECT_FLOAT_EQ(10.f, rs->GetMaximum(3));
    EXPECT_FLOAT_EQ(8.502796f, rs->GetAverage(3));
    EXPECT_FLOAT_EQ(1.382485f, rs->GetStd(3));
    EXPECT_FLOAT_EQ(9.4f, rs->GetRange(3));

    EXPECT_TRUE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->GetMask()); // m_mask

    /** Test getting position data **/
    int ncells = -1;
    int** positions = nullptr;
    rs->GetRasterPositionData(&ncells, &positions); // m_rasterPositionData
    EXPECT_EQ(545, ncells);
    EXPECT_NE(nullptr, positions);
    // index = 0, row = 0 and col = 1
    EXPECT_EQ(0, positions[0][0]);
    EXPECT_EQ(1, positions[0][1]);
    // index = 544, row = 19 and col = 29
    EXPECT_EQ(19, positions[544][0]);
    EXPECT_EQ(29, positions[544][1]);

    /** Test getting raster data **/
    ncells = 0;
    float* rs_data = nullptr;
    EXPECT_FALSE(rs->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_TRUE(rs->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(545, ncells);
    EXPECT_EQ(3, nlyrs);
    EXPECT_NE(nullptr, rs_2ddata);
    // raster layer 1
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[0][0]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][0]);
    EXPECT_FLOAT_EQ(7.14f, rs_2ddata[4][0]);
    // raster layer 2
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[0][1]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][1]);
    EXPECT_FLOAT_EQ(27.14f, rs_2ddata[4][1]);
    // raster layer 3
    EXPECT_FLOAT_EQ(1.9f, rs_2ddata[0][2]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][2]);
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[4][2]);

    /** Get raster cell value by various way **/
    // invalid inputs which return nodata
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(545));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(545, 4));
    // valid inputs
    EXPECT_FLOAT_EQ(9.9f, rs->GetValueByIndex(0)); // layer 1 by default
    EXPECT_FLOAT_EQ(7.21f, rs->GetValueByIndex(544, 1));
    EXPECT_FLOAT_EQ(7.14f, rs->GetValueByIndex(4, 1));   // layer 1
    EXPECT_FLOAT_EQ(27.14f, rs->GetValueByIndex(4, 2));  // layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(4, 3)); // layer 3

    int tmp_lyr;
    float* tmp_values;
    rs->GetValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValueByIndex(4, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.14f, tmp_values[0]);
    EXPECT_FLOAT_EQ(27.14f, tmp_values[1]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[2]);

    // Get value by row, col, and layer (optional)
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, 4));

    EXPECT_FLOAT_EQ(8.06f, rs->GetValue(2, 4));
    EXPECT_FLOAT_EQ(8.06f, rs->GetValue(2, 4, 1));
    EXPECT_FLOAT_EQ(8.06f, rs->GetValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.06f, rs->GetValue(2, 4, 3));

    rs->GetValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[1]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[2]);
    rs->GetValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[1]);
    EXPECT_FLOAT_EQ(1.9f, tmp_values[2]);

    // Get position
    EXPECT_EQ(29, rs->GetPosition(4.05f, 37.95f));
    EXPECT_EQ(29, rs->GetPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string corename = rs->GetCoreName();
    string newcorename = corename + "_2D-pos-nomask";
    rs->SetCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->GetCoreName());

    // Set raster data value
    rs->SetValue(2, 4, 0.806f);
    EXPECT_FLOAT_EQ(0.806f, rs->GetValue(2, 4));
    rs->SetValue(2, 4, 0.806f, 2);
    EXPECT_FLOAT_EQ(0.806f, rs->GetValue(2, 4, 2));
    rs->SetValue(2, 4, 0.806f, 3);
    EXPECT_FLOAT_EQ(0.806f, rs->GetValue(2, 4, 3));
    rs->SetValue(2, 4, 0.806f, 4);
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, 4));
    rs->SetValue(0, 0, 1.f); // current version do not support setting value to NODATA location
    EXPECT_NE(1.f, rs->GetValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, 0));

    // update statistics
    rs->UpdateStatistics(); // Should be manually invoked in your project!
    // layer 1
    EXPECT_FLOAT_EQ(0.806f, rs->GetMinimum(1));
    EXPECT_FLOAT_EQ(8.68065321f, rs->GetAverage(1));
    EXPECT_FLOAT_EQ(0.93353500f, rs->GetStd(1));
    EXPECT_FLOAT_EQ(9.194f, rs->GetRange(1));
    // layer 2
    EXPECT_FLOAT_EQ(0.806f, rs->GetMinimum(2));
    EXPECT_FLOAT_EQ(9.19171165f, rs->GetAverage(2));
    EXPECT_FLOAT_EQ(5.62426552f, rs->GetStd(2));
    EXPECT_FLOAT_EQ(97.684f, rs->GetRange(2));
    // layer 3
    EXPECT_FLOAT_EQ(8.48936296f, rs->GetAverage(3));
    EXPECT_FLOAT_EQ(1.42141729f, rs->GetStd(3));

    /** Output to new file **/
    string oldfullname = rs->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    string newfullname4mongo = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "_mongo." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->OutputToFile(newfullname));

#ifdef USE_MONGODB
    /** MongoDB I/O test **/
    MongoClient* conn = MongoClient::Init("127.0.0.1", 27017);
    if (nullptr != conn) {
        string gfsfilename = newcorename + "_" + GetSuffix(oldfullname);
        MongoGridFs* gfs = new MongoGridFs(conn->GetGridFs("test", "spatial"));
        gfs->RemoveFile(gfsfilename);
        rs->OutputToMongoDB(gfsfilename, gfs);
        clsRasterData<float>* mongors = clsRasterData<float>::Init(gfs, gfsfilename.c_str());
        // test mongors data
        EXPECT_EQ(545, mongors->GetCellNumber()); // m_nCells
        EXPECT_EQ(3, mongors->GetLayers());
        EXPECT_EQ(545, mongors->GetValidNumber());
        // layer 1
        EXPECT_FLOAT_EQ(0.806f, rs->GetMinimum(1));
        EXPECT_FLOAT_EQ(8.68065321f, rs->GetAverage(1));
        EXPECT_FLOAT_EQ(0.93353500f, rs->GetStd(1));
        EXPECT_FLOAT_EQ(9.194f, rs->GetRange(1));
        // layer 2
        EXPECT_FLOAT_EQ(0.806f, rs->GetMinimum(2));
        EXPECT_FLOAT_EQ(9.19171165f, rs->GetAverage(2));
        EXPECT_FLOAT_EQ(5.62426552f, rs->GetStd(2));
        EXPECT_FLOAT_EQ(97.684f, rs->GetRange(2));
        // layer 3
        EXPECT_FLOAT_EQ(8.48936296f, rs->GetAverage(3));
        EXPECT_FLOAT_EQ(1.42141729f, rs->GetStd(3));
        // output to asc/tif file for comparison
        EXPECT_TRUE(mongors->OutputToFile(newfullname4mongo));
    }
#endif
}
} /* namespace */
