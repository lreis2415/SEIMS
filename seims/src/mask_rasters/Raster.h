/*----------------------------------------------------------------------
*	Purpose: 	Raster Data
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/

#ifndef NEW_WETSPA_RASTER_INCLUDE
#define NEW_WETSPA_RASTER_INCLUDE

#include "gdal.h"
#include <string>

using namespace std;
#define RASTER_MINI_VALUE 0.000001

template<typename T>
class Raster
{
public:
    Raster(void);

    ~Raster(void);

    int ReadAsInt32(const char *filename);

    int ReadAsFloat32(const char *filename);

    int CopyMask(Raster<int> &mask);

    void SetValue(int i, int j, T val) { m_data[i][j] = val; }

    int GetNumberOfRows() { return m_nRows; }

    int GetNumberofColumns() { return m_nCols; }

    double GetXMin() { return m_xMin; }

    double GetYMax() { return m_yMax; }

    double GetXCellSize() { return m_dx; }

    double GetYCellSize() { return m_dy; }

    T GetNoDataValue() { return (T) m_noDataValue; }

    string &GetProjection() { return m_proj; }

    GDALDataType GetDataType() { return m_dType; }

    void SetDataType(GDALDataType dType) { m_dType = dType; }

    T *GetData() { return m_data; }

    void OutputGTiff(const char *rasterName);

    void OutputAsc(const char *rasterName);

private:
    T *m_data;
    double m_noDataValue;
    int m_nRows, m_nCols, m_nAll;
    double m_dx, m_dy;
    double m_xMin, m_yMax;

    GDALDataType m_dType;
    string m_proj;

public:
    bool IsNull(int i);

    bool IsEmpty(void);
};

#endif
