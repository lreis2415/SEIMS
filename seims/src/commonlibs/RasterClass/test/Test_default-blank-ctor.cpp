/*!
 * @brief Test clsRasterData of blank constructor to make sure no exception thrown.
 *
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/04/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

namespace {
TEST(clsRasterDataTestBlankCtor, ValidateAccess) {
    /// 0. Create an clsRasterData instance with blank ctor
    clsRasterData<float, int> *rs = new clsRasterData<float, int>();
    /// 1. Test members after constructing.
    EXPECT_EQ(-1, rs->getDataLength());  // m_nCells
    EXPECT_EQ(-1, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    EXPECT_EQ("", rs->getFilePath());  // m_filePathName
    EXPECT_EQ("", rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_FALSE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_FALSE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_FALSE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    EXPECT_FALSE(rs->validate_raster_data());

    EXPECT_EQ(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_EQ(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    EXPECT_EQ(-9999, rs->getRows());
    EXPECT_EQ(-9999, rs->getCols());
    EXPECT_FLOAT_EQ(-9999.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(-9999.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(-9999.f, rs->getCellWidth());
    EXPECT_EQ(-1, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(-9999, rs->getValidNumber());
    EXPECT_FLOAT_EQ(-9999.f, rs->getMinimum());
    EXPECT_FLOAT_EQ(-9999.f, rs->getMaximum());
    EXPECT_FLOAT_EQ(-9999.f, rs->getAverage());
    EXPECT_FLOAT_EQ(-9999.f, rs->getSTD());
    EXPECT_FLOAT_EQ(-9999.f, rs->getRange());
    EXPECT_FALSE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->getMask());  // m_mask

    /** Test getting position data **/
    int ncells = -1;
    int **positions = nullptr;
    rs->getRasterPositionData(&ncells, &positions);  // m_rasterPositionData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, positions);

    /** Test getting raster data **/
    float *rs_data = nullptr;
    EXPECT_FALSE(rs->getRasterData(&ncells, &rs_data));  // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float **rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs->get2DRasterData(&ncells, &nlyrs, &rs_2ddata));  // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(540, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(541, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(541, 2));

    int tmp_lyr;
    float *tmp_values;
    rs->getValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValueByIndex(0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);

    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(20, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 1));

    rs->getValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);

    // Get position
    EXPECT_EQ(-2, rs->getPosition(4.05f, 37.95f));
    EXPECT_EQ(-2, rs->getPosition(5.95f, 36.05f));

    /** Set value **/

    // Set raster data value
    rs->setValue(2, 4, 18.06f);
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4));
    rs->setValue(0, 0, 1.f);
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 0));

    /** Output to new file **/
    string newfullname = GetAppPath() + SEP + "no_output.tif";
    EXPECT_FALSE(rs->outputToFile(newfullname));
}
} /* namespace */