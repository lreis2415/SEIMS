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

#include "gridfs.h"

template<typename T>
class Raster
{
public:
    Raster(void);

    ~Raster(void);

    int ReadArcAscHeader(const char *filename);

    int ReadMongoDBHeader(gridfs *gfs, const char *remoteFilename);

    int ReadFromArcAsc(const char *filename);

    int ReadFromMWAsc(const char *filename);

    int ReadFromGDAL(const char *filename);

    int ReadFromMongoDB(gridfs *gfs, const char *remoteFilename);

    int Copy(Raster &otherRaster);

    int OutputMWAscii(const char *filename);

    int OutputArcAscii(const char *filename);

    void SetZero();

    void SetDefaultValues(T value);

    void Add(Raster &otherRaster);

    T At(int i, int j) { return m_data[i][j]; }

    void SetValue(int i, int j, T val) { m_data[i][j] = val; }

    int GetNumberOfRows() { return m_nRows; }

    int GetNumberofColumns() { return m_nCols; }

    float GetXllCenter() { return m_xllCenter; }

    float GetYllCenter() { return m_yllCenter; }

    float GetXCellSize() { return m_dx; }

    float GetYCellSize() { return m_dy; }

    T GetNoDataValue() { return m_noDataValue; }

    T **GetData() { return m_data; }

    static void OutputRaster(const char *rasterName, int nRows, int nCols, T xll, T yll, T dx, T noDataValue, T **data);

    static void OutputGTiff(const char *rasterName, int nRows, int nCols, T xll, T yll, T dx, T noDataValue, T *data);

private:
    T **m_data;
    T m_noDataValue;
    int m_nRows, m_nCols;
    float m_xllCenter, m_yllCenter;
    float m_dx, m_dy;

private:
    void DeleteExistingData(void);

public:
    bool IsNull(int i, int j);

    bool IsEmpty(void);
};

#endif