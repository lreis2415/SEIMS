/*!
 * \brief Test clsRasterData of blank constructor to make sure no exception thrown.
 *
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

namespace {
TEST(clsRasterDataTestBlankCtor, ValidateAccess) {
    /// 0. Create an clsRasterData instance with blank ctor
    clsRasterData<float, int>* rs = new clsRasterData<float, int>();
    /// 1. Test members after constructing.
    EXPECT_EQ(-1, rs->GetDataLength()); // m_nCells
    EXPECT_EQ(-1, rs->GetCellNumber()); // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->GetNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->GetDefaultValue()); // m_defaultValue

    EXPECT_EQ("", rs->GetFilePath()); // m_filePathName
    EXPECT_EQ("", rs->GetCoreName()); // m_coreFileName

    EXPECT_TRUE(rs->Initialized());           // m_initialized
    EXPECT_FALSE(rs->Is2DRaster());           // m_is2DRaster
    EXPECT_FALSE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());   // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());         // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated()); // m_statisticsCalculated

    EXPECT_FALSE(rs->ValidateRasterData());

    EXPECT_EQ(nullptr, rs->GetRasterDataPointer());         // m_rasterData
    EXPECT_EQ(nullptr, rs->Get2DRasterDataPointer());       // m_raster2DData
    EXPECT_EQ(nullptr, rs->GetRasterPositionDataPointer()); // m_rasterPositionData

    /** Get metadata, m_headers **/
    EXPECT_EQ(-9999, rs->GetRows());
    EXPECT_EQ(-9999, rs->GetCols());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetXllCenter());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetYllCenter());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetCellWidth());
    EXPECT_EQ(-1, rs->GetLayers());
    EXPECT_STREQ("", rs->GetSrs());
    EXPECT_EQ("", rs->GetSrsString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(-9999, rs->GetValidNumber());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetMinimum());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetMaximum());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetAverage());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetStd());
    EXPECT_FLOAT_EQ(-9999.f, rs->GetRange());
    EXPECT_FALSE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->GetMask()); // m_mask

    /** Test getting position data **/
    int ncells = -1;
    int** positions = nullptr;
    rs->GetRasterPositionData(&ncells, &positions); // m_rasterPositionData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, positions);

    /** Test getting raster data **/
    float* rs_data = nullptr;
    EXPECT_FALSE(rs->GetRasterData(&ncells, &rs_data)); // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs->Get2DRasterData(&ncells, &nlyrs, &rs_2ddata)); // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(540, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(541, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(29));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValueByIndex(541, 2));

    int tmp_lyr;
    float* tmp_values;
    rs->GetValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValueByIndex(0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);

    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(20, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4));
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4, 1));

    rs->GetValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->GetValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position
    EXPECT_EQ(-2, rs->GetPosition(4.05f, 37.95f));
    EXPECT_EQ(-2, rs->GetPosition(5.95f, 36.05f));

    /** Set value **/

    // Set raster data value
    rs->SetValue(2, 4, 18.06f);
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(2, 4));
    rs->SetValue(0, 0, 1.f);
    EXPECT_FLOAT_EQ(-9999.f, rs->GetValue(0, 0));

    /** Output to new file **/
    string newfullname = ccgl::utils_filesystem::GetAppPath() + SEP + "no_output.tif";
    EXPECT_FALSE(rs->OutputToFile(newfullname));
}
} /* namespace */
