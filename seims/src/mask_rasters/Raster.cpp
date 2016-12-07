/*----------------------------------------------------------------------
*	Purpose: 	Raster Data
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
#ifndef SEIMS_RASTER_CPP_INCLUDE
#define SEIMS_RASTER_CPP_INCLUDE

#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include "Raster.h"
//gdal
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include <iomanip>
#include "ogr_spatialref.h"

#endif
using namespace std;

template<typename T>
inline Raster<T>::Raster(void) : m_data(NULL), m_nRows(0), m_nCols(0),
                                 m_xMin(0), m_yMax(0), m_dx(0), m_dy(0), m_noDataValue(-9999), m_dType(GDT_Float32)
{
}

template<typename T>
inline Raster<T>::~Raster(void)
{
    if (m_data != NULL)
        CPLFree(m_data);
}

template<typename T>
int Raster<T>::ReadAsInt32(const char *filename)
{
    GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
    if (poDataset == NULL)
    {
        cerr << "Open file: " << filename << " failed.\n";
        return -1;
    }

    GDALRasterBand *poBand = poDataset->GetRasterBand(1);
    m_nCols = poBand->GetXSize();
    m_nRows = poBand->GetYSize();
    m_nAll = m_nRows * m_nCols;

    m_noDataValue = poBand->GetNoDataValue();
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    m_dx = adfGeoTransform[1];
    m_dy = -adfGeoTransform[5];
    m_xMin = adfGeoTransform[0];
    m_yMax = adfGeoTransform[3];

    m_proj = poDataset->GetProjectionRef();

    if (m_data != NULL)
        CPLFree(m_data);

    m_dType = GDT_Int32;
    m_data = (T *) CPLMalloc(sizeof(int) * m_nAll);

    poBand->RasterIO(GF_Read, 0, 0, m_nCols, m_nRows, m_data, m_nCols, m_nRows, m_dType, 0, 0);
    GDALClose(poDataset);

    return 0;
}

template<typename T>
int Raster<T>::ReadAsFloat32(const char *filename)
{
    GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
    if (poDataset == NULL)
    {
        cerr << "Open file: " << filename << " failed.\n";
        return -1;
    }

    GDALRasterBand *poBand = poDataset->GetRasterBand(1);
    m_nCols = poBand->GetXSize();
    m_nRows = poBand->GetYSize();
    m_nAll = m_nRows * m_nCols;

    m_noDataValue = poBand->GetNoDataValue();
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    m_dx = adfGeoTransform[1];
    m_dy = -adfGeoTransform[5];
    m_xMin = adfGeoTransform[0];
    m_yMax = adfGeoTransform[3];

    m_proj = poDataset->GetProjectionRef();

    if (m_data != NULL)
        CPLFree(m_data);

    m_dType = GDT_Float32;

    m_data = (T *) CPLMalloc(sizeof(float) * m_nAll);

    poBand->RasterIO(GF_Read, 0, 0, m_nCols, m_nRows, m_data, m_nCols, m_nRows, m_dType, 0, 0);
    GDALClose(poDataset);

    return 0;
}

template<typename T>
void Raster<T>::OutputGTiff(const char *rasterName)
{
    const char *pszFormat = "GTiff";
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    char **papszOptions = poDriver->GetMetadata();
    //papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
    //papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "PACKBITS" );
    GDALDataset *poDstDS = poDriver->Create(rasterName, m_nCols, m_nRows, 1, m_dType, papszOptions);

    //write the data to new file
    GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
    poDstBand->RasterIO(GF_Write, 0, 0, m_nCols, m_nRows, m_data, m_nCols, m_nRows, m_dType, 0, 0);
    poDstBand->SetNoDataValue(m_noDataValue);

    double geoTrans[6];
    geoTrans[0] = m_xMin;
    geoTrans[1] = m_dx;
    geoTrans[2] = 0;
    geoTrans[3] = m_yMax;
    geoTrans[4] = 0;
    geoTrans[5] = -m_dy;
    poDstDS->SetGeoTransform(geoTrans);

    poDstDS->SetProjection(m_proj.c_str());

    GDALClose(poDstDS);
}

template<typename T>
void Raster<T>::OutputAsc(const char *rasterName)
{
    ofstream rasterFile(rasterName);
    rasterFile << "NCOLS " << m_nCols << "\n";
    rasterFile << "NROWS " << m_nRows << "\n";
    rasterFile << "XLLCORNER " << m_xMin << "\n";
    rasterFile << "YLLCORNER " << m_yMax - m_dy * m_nRows << "\n";
    rasterFile << "CELLSIZE " << m_dx << "\n";
    rasterFile << "NODATA_VALUE " << m_noDataValue << "\n";

    //write file
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            rasterFile << m_data[i * m_nCols + j] << "\t";
        }
        rasterFile << "\n";
    }

    rasterFile.close();
}

template<typename T>
int Raster<T>::CopyMask(Raster<int> &otherRaster)
{
    m_nRows = otherRaster.GetNumberOfRows();
    m_nCols = otherRaster.GetNumberofColumns();
    m_xMin = otherRaster.GetXMin();
    m_yMax = otherRaster.GetYMax();
    m_dx = otherRaster.GetXCellSize();
    m_dy = otherRaster.GetYCellSize();
    //m_noDataValue = otherRaster.GetNoDataValue();
    m_nAll = m_nRows * m_nCols;

    m_proj = otherRaster.GetProjection();
    //allocate memory
    if (m_data != NULL)
        CPLFree(m_data);
    m_data = (T *) CPLMalloc(sizeof(T) * m_nAll);

    return 0;
}

template<typename T>
inline bool Raster<T>::IsNull(int i)
{
    return abs(m_data[i] - m_noDataValue) < RASTER_MINI_VALUE;
}

template<typename T>
inline bool Raster<T>::IsEmpty(void)
{
    return (m_data == NULL);
}


