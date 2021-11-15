/*!
 * \brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:      NO             --            --               NO
 *        Mask data  :      --             --            --               --
 *
 *        TEST CASE NAME (or TEST SUITE):
 *            clsRasterDataTestMultiNoPosNoMask
 *
 *        Since the core code is irrelevant with the format of raster data, we take ASC
 *        as example here.
 * \version 1.2
 * \authors Liangjun Zhu (zlj@lreis.ac.cn)
 * \revised 2017-12-02 - lj - Original version.
 *          2018-05-03 - lj - Integrated into CCGL.
 *          2021-07-20 - lj - Update after changes of GetValue and GetValueByIndex.
 *
 */
#include "gtest/gtest.h"
#include "../../src/data_raster.hpp"
#include "../../src/utils_filesystem.h"
#include "../../src/utils_array.h"

using namespace ccgl::data_raster;
using namespace ccgl::utils_filesystem;
using namespace ccgl::utils_array;

namespace {

TEST(clsRasterDataTestMultiNoPosNoMask, RasterIO) {
    /// 0. Read multiple raster data.
    string apppath = GetAppPath();
    vector<string> filenames;
    filenames.emplace_back(apppath + "./data/raster/dem_1.asc");
    filenames.emplace_back(apppath + "./data/raster/dem_2.asc");
    filenames.emplace_back(apppath + "./data/raster/dem_3.asc");
    clsRasterData<float>* rs = clsRasterData<float>::Init(filenames, false); // recommended way
    //clsRasterData<float> *rs = new clsRasterData<float>(filenames, false);  // unsafe way

    ASSERT_NE(nullptr, rs);

    /// 1. Test members after constructing.
    EXPECT_EQ(600, rs->GetDataLength()); // m_nCells, which will be nRows * nCols
    EXPECT_EQ(600, rs->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->GetDefaultValue()); // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ("dem", rs->GetCoreName());                     // m_coreFileName
    EXPECT_EQ("dem_%d", GetCoreFileName(rs->GetFilePath())); // m_filePathName

    EXPECT_TRUE(rs->Initialized());           // m_initialized
    EXPECT_TRUE(rs->Is2DRaster());            // m_is2DRaster
    EXPECT_FALSE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());   // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated()); // m_statisticsCalculated

    ASSERT_TRUE(rs->ValidateRasterData());

    EXPECT_EQ(nullptr, rs->GetRasterDataPointer());         // m_rasterData
    EXPECT_NE(nullptr, rs->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs->GetRasterPositionDataPointer()); // m_rasterPositionData

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

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_FALSE(rs->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_TRUE(rs->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(600, ncells);
    EXPECT_EQ(3, nlyrs);
    EXPECT_NE(nullptr, rs_2ddata);
    // raster layer 1
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][0]);
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[1][0]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[599][0]);
    EXPECT_FLOAT_EQ(7.14f, rs_2ddata[5][0]);
    // raster layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][1]);
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[1][1]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[599][1]);
    EXPECT_FLOAT_EQ(27.14f, rs_2ddata[5][1]);
    // raster layer 3
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[0][2]);
    EXPECT_FLOAT_EQ(1.9f, rs_2ddata[1][2]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[599][2]);
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[5][2]);

    /** Get raster cell value by various way **/
    // invalid inputs which return nodata
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(600));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(600, 4));
    // valid inputs
    EXPECT_FLOAT_EQ(9.9f, rs->GetValueByIndex(1)); // layer 1 by default
    EXPECT_FLOAT_EQ(7.21f, rs->GetValueByIndex(599, 1));
    EXPECT_FLOAT_EQ(7.14f, rs->GetValueByIndex(5, 1));   // layer 1
    EXPECT_FLOAT_EQ(27.14f, rs->GetValueByIndex(5, 2));  // layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(5, 3)); // layer 3

    int tmp_lyr = rs->GetLayers();
    float* tmp_values = nullptr;
    Initialize1DArray(tmp_lyr, tmp_values, -9999.f);
    rs->GetValueByIndex(-1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValueByIndex(5, tmp_values);
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

    rs->GetValue(-1, 0, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, -1, tmp_values);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, 0, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[1]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[2]);
    rs->GetValue(0, 1, tmp_values);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[1]);
    EXPECT_FLOAT_EQ(1.9f, tmp_values[2]);

    Release1DArray(tmp_values);

    // Get position
    EXPECT_EQ(32, rs->GetPosition(4.05f, 37.95f));
    EXPECT_EQ(32, rs->GetPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string corename = rs->GetCoreName();
    string newcorename = corename + "_2D-nopos-nomask";
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
    rs->SetValue(0, 0, 1.f);
    EXPECT_FLOAT_EQ(1.f, rs->GetValue(0, 0, 1));


    // update statistics
    rs->UpdateStatistics(); // Should be manually invoked in your project!
    // layer 1
    EXPECT_FLOAT_EQ(0.806, rs->GetMinimum(1));
    EXPECT_FLOAT_EQ(8.66658608, rs->GetAverage(1));
    EXPECT_FLOAT_EQ(0.98880652, rs->GetStd(1));
    EXPECT_FLOAT_EQ(9.194, rs->GetRange(1));
    // layer 2
    EXPECT_FLOAT_EQ(0.806, rs->GetMinimum(2));
    EXPECT_FLOAT_EQ(9.19171165, rs->GetAverage(2));
    EXPECT_FLOAT_EQ(5.62426552, rs->GetStd(2));
    EXPECT_FLOAT_EQ(97.684, rs->GetRange(2));
    // layer 3
    EXPECT_FLOAT_EQ(8.48936296f, rs->GetAverage(3));
    EXPECT_FLOAT_EQ(1.42141729f, rs->GetStd(3));

    /** Output to new file **/
    string oldfullname = rs->GetFilePath();
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
            newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->OutputToFile(newfullname));

    /* Get position data, which will be calculated if not existed */
    ncells = -1;
    int** positions = nullptr;
    rs->GetRasterPositionData(&ncells, &positions); // m_rasterPositionData
    EXPECT_TRUE(rs->PositionsCalculated());
    EXPECT_TRUE(rs->PositionsAllocated());
    EXPECT_NE(nullptr, rs->GetRasterPositionDataPointer());
    EXPECT_EQ(546, ncells);
    EXPECT_NE(nullptr, positions);
    // index = 0, row = 0 and col = 0
    EXPECT_EQ(0, positions[0][0]);
    EXPECT_EQ(0, positions[0][1]);
    // index = 545, row = 19 and col = 29
    EXPECT_EQ(19, positions[545][0]);
    EXPECT_EQ(29, positions[545][1]);

    // In the meantime, other related variables are updated, test some of them.
    EXPECT_EQ(546, rs->GetCellNumber()); // m_nCells
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(1.f, rs->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(7.21f, rs->GetValueByIndex(545, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(546, 1));
    EXPECT_FLOAT_EQ(9.43f, rs->GetValueByIndex(30));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1, 2));

    // Release resources
    delete rs;
}
} /* namespace */
