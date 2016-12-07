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
#include "util.h"

//gdal
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"

#include "mongo.h"
#include "bson.h"

#include <iomanip>

#include "ogr_spatialref.h"

using namespace std;

template<typename T>
inline Raster<T>::Raster(void) : m_data(NULL), m_nRows(0), m_nCols(0),
                                 m_xllCenter(0), m_yllCenter(0), m_dx(0), m_dy(0), m_noDataValue(-9999)
{
}

template<typename T>
inline Raster<T>::~Raster(void)
{
    DeleteExistingData();
}

template<typename T>
inline int Raster<T>::ReadArcAscHeader(const char *filename)
{
    ifstream rasterFile(filename);
    string tmp;

    bool isCorner = false;
    //read header
    rasterFile >> tmp >> m_nCols;
    rasterFile >> tmp >> m_nRows;
    rasterFile >> setprecision(8) >> tmp >> m_xllCenter;
    if (StringMatch(tmp, "xllcorner"))
        isCorner = true;
    rasterFile >> tmp >> m_yllCenter;
    rasterFile >> tmp >> m_dx;
    m_dy = m_dx;
    rasterFile >> tmp >> m_noDataValue;

    if (isCorner)
    {
        m_xllCenter += 0.5f * m_dx;
        m_yllCenter += 0.5f * m_dy;
    }

    rasterFile.close();

    return 0;
}

template<typename T>
inline int Raster<T>::ReadFromArcAsc(const char *filename)
{
    ifstream rasterFile(filename);
    string tmp;

    bool isCorner = false;
    //read header
    rasterFile >> tmp >> m_nCols;
    rasterFile >> tmp >> m_nRows;
    rasterFile >> setprecision(8) >> tmp >> m_xllCenter;
    if (StringMatch(tmp, "xllcorner"))
        isCorner = true;
    rasterFile >> tmp >> m_yllCenter;
    rasterFile >> tmp >> m_dx;
    m_dy = m_dx;
    rasterFile >> tmp >> m_noDataValue;

    if (isCorner)
    {
        m_xllCenter += 0.5f * m_dx;
        m_yllCenter += 0.5f * m_dy;
    }

    //allocate memory
    DeleteExistingData();
    m_data = new T *[m_nRows];
    for (int i = 0; i < m_nRows; ++i)
    {
        m_data[i] = new T[m_nCols];
    }

    //read file
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            rasterFile >> m_data[i][j];
        }
    }

    rasterFile.close();

    return 0;
}

template<typename T>
int Raster<T>::ReadFromMWAsc(const char *filename)
{
    ifstream rasterFile(filename);
    string tmp;
    //read header
    rasterFile >> tmp >> m_nCols;
    rasterFile >> tmp >> m_nRows;
    rasterFile >> tmp >> m_xllCenter;
    rasterFile >> tmp >> m_yllCenter;
    rasterFile >> tmp >> m_dx;
    rasterFile >> tmp >> m_dy;
    rasterFile >> tmp >> m_noDataValue;

    //allocate memory
    DeleteExistingData();
    m_data = new T *[m_nRows];
    for (int i = 0; i < m_nRows; ++i)
    {
        m_data[i] = new T[m_nCols];
    }

    //read file
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            rasterFile >> m_data[i][j];
        }
    }

    rasterFile.close();

    return 0;
}

template<typename T>
int Raster<T>::ReadFromMongoDB(gridfs *gfs, const char *remoteFilename)
{
    //mongo conn[1];
    //gridfs gfs[1];
    //int status = mongo_connect(conn, host, port);
    //if( MONGO_OK != status )
    //{
    //	cout << "can not connect to mongodb, host: " << host << ", port: " << port << "\n";
    //	return -1;
    //}
    //gridfs_init(conn, "model_1", "spatial", gfs);

    gridfile gfile[1];
    bson b[1];
    bson_init(b);
    bson_append_string(b, "filename", remoteFilename);
    bson_finish(b);
    int flag = gridfs_find_query(gfs, b, gfile);
    if (0 != flag)
    {
        cout << "Failed in ReadFromMongoDB, Remote file: " << remoteFilename << endl;
        return -1;
    }

    size_t length = (size_t) gridfile_get_contentlength(gfile);
    char *buf = (char *) malloc(length);
    gridfile_read(gfile, length, buf);
    float *data = (float *) buf;

    bson bmeta[1];
    gridfile_get_metadata(gfile, bmeta);
    bson_iterator iterator[1];
    if (bson_find(iterator, bmeta, "NCOLS"))
        m_nCols = bson_iterator_int(iterator);
    if (bson_find(iterator, bmeta, "NROWS"))
        m_nRows = bson_iterator_int(iterator);
    if (bson_find(iterator, bmeta, "NODATA_VALUE"))
        m_noDataValue = (T) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "XLLCENTER"))
        m_xllCenter = (float) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "YLLCENTER"))
        m_yllCenter = (float) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "CELLSIZE"))
    {
        m_dx = (float) bson_iterator_double(iterator);
        m_dy = m_dx;
    }
    //bson_destroy(bmeta);

    //allocate memory
    DeleteExistingData();
    m_data = new T *[m_nRows];
    for (int i = 0; i < m_nRows; ++i)
    {
        m_data[i] = new T[m_nCols];
    }

    //read file
    int index = 0;
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            index = i * m_nCols + j;
            m_data[i][j] = T(data[index]);
        }
    }

    bson_destroy(b);
    gridfile_destroy(gfile);
    //gridfs_destroy(gfs);
    //mongo_destroy(conn);
    free(buf);
    return 0;
}

template<typename T>
int Raster<T>::ReadMongoDBHeader(gridfs *gfs, const char *remoteFilename)
{
    //mongo conn[1];
    //gridfs gfs[1];
    //int status = mongo_connect(conn, host, port);
    //if( MONGO_OK != status )
    //{
    //	cout << "can not connect to mongodb, host: " << host << ", port: " << port << "\n";
    //	return -1;
    //}
    //gridfs_init(conn, "model_1", "spatial", gfs);

    gridfile gfile[1];
    bson b[1];
    bson_init(b);
    bson_append_string(b, "filename", remoteFilename);
    bson_finish(b);
    int flag = gridfs_find_query(gfs, b, gfile);
    if (0 != flag)
    {
        cout << "Failed in ReadMongoDBHeader, Remote file: " << remoteFilename << endl;
        return -1;
    }

    bson bmeta[1];
    gridfile_get_metadata(gfile, bmeta);
    bson_iterator iterator[1];
    if (bson_find(iterator, bmeta, "NCOLS"))
        m_nCols = bson_iterator_int(iterator);
    if (bson_find(iterator, bmeta, "NROWS"))
        m_nRows = bson_iterator_int(iterator);
    if (bson_find(iterator, bmeta, "NODATA_VALUE"))
        m_noDataValue = (T) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "XLLCENTER"))
        m_xllCenter = (float) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "YLLCENTER"))
        m_yllCenter = (float) bson_iterator_double(iterator);
    if (bson_find(iterator, bmeta, "CELLSIZE"))
    {
        m_dx = (float) bson_iterator_double(iterator);
        m_dy = m_dx;
    }

    bson_destroy(b);
    gridfile_destroy(gfile);
    return 0;
}


template<typename T>
int Raster<T>::ReadFromGDAL(const char *filename)
{
    GDALDataset *poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
    if (poDataset == NULL)
    {
        cerr << "Open subbasin file failed.\n";
        return -1;
    }

    GDALRasterBand *poBand = poDataset->GetRasterBand(1);
    m_nCols = poBand->GetXSize();
    m_nRows = poBand->GetYSize();
    m_noDataValue = (float) poBand->GetNoDataValue();
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    m_dx = adfGeoTransform[1];
    m_dy = -adfGeoTransform[5];
    m_xllCenter = adfGeoTransform[0] + 0.5f * m_dx;
    m_yllCenter = adfGeoTransform[3] - (m_nRows - 0.5f) * m_dy;

    //allocate memory
    DeleteExistingData();
    m_data = new T *[m_nRows];
    for (int i = 0; i < m_nRows; ++i)
    {
        m_data[i] = new T[m_nCols];
    }

    float *pData = (float *) CPLMalloc(sizeof(float) * m_nCols * m_nRows);
    poBand->RasterIO(GF_Read, 0, 0, m_nCols, m_nRows,
                     pData, m_nCols, m_nRows, GDT_Float32, 0, 0);

    for (int i = 0; i < m_nRows; i++)
    {
        for (int j = 0; j < m_nCols; j++)
        {
            int index = i * m_nCols + j;
            m_data[i][j] = T(pData[index]);
        }
    }

    GDALClose(poDataset);

    return 0;
}

template<typename T>
int Raster<T>::OutputMWAscii(const char *filename)
{
    ofstream rasterFile(filename);
    //write header
    rasterFile << "NCOLS " << m_nCols << "\n";
    rasterFile << "NROWS " << m_nRows << "\n";
    rasterFile << "XLLCENTER " << m_xllCenter << "\n";
    rasterFile << "YLLCENTER " << m_yllCenter << "\n";
    rasterFile << "DX " << m_dx << "\n";
    rasterFile << "DY " << m_dy << "\n";
    rasterFile << "NODATA_VALUE " << m_noDataValue << "\n";

    //write file
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            rasterFile << m_data[i][j] << " ";
        }
        rasterFile << "\n";
    }

    rasterFile.close();

    return 0;
}

template<typename T>
int Raster<T>::OutputArcAscii(const char *filename)
{
    ofstream rasterFile(filename);
    //write header
    rasterFile << "NCOLS " << m_nCols << "\n";
    rasterFile << "NROWS " << m_nRows << "\n";
    rasterFile << "XLLCENTER " << m_xllCenter << "\n";
    rasterFile << "YLLCENTER " << m_yllCenter << "\n";
    rasterFile << "CELLSIZE " << m_dx << "\n";
    rasterFile << "NODATA_VALUE " << m_noDataValue << "\n";

    //write file
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            rasterFile << m_data[i][j] << " ";
        }
        rasterFile << "\n";
    }

    rasterFile.close();

    return 0;
}

template<typename T>
void Raster<T>::OutputRaster(const char *rasterName, int nRows, int nCols, T xll, T yll, T dx, T noDataValue, T **data)
{
    ofstream rasterFile(rasterName);
    //write header
    rasterFile << "NCOLS " << nCols << "\n";
    rasterFile << "NROWS " << nRows << "\n";
    rasterFile << "XLLCORNER " << xll << "\n";
    rasterFile << "YLLCORNER " << yll << "\n";
    rasterFile << "CELLSIZE " << dx << "\n";
    rasterFile << "NODATA_VALUE " << noDataValue << "\n";

    //write file
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
        {
            rasterFile << data[i][j] << " ";
        }
        rasterFile << "\n";
    }

    rasterFile.close();

}

template<typename T>
void Raster<T>::OutputGTiff(const char *rasterName, int nRows, int nCols, T xll, T yll, T dx, T noDataValue, T *data)
{
    const char *pszFormat = "GTiff";
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    char **papszOptions = poDriver->GetMetadata();
    GDALDataset *poDstDS = poDriver->Create(rasterName, nCols, nRows, 1, GDT_Float32, papszOptions);

    //write the data to new file
    GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
    poDstBand->RasterIO(GF_Write, 0, 0, nCols, nRows, data, nCols, nRows, GDT_Float32, 0, 0);
    poDstBand->SetNoDataValue(noDataValue);

    double geoTrans[6];
    geoTrans[0] = xll;
    geoTrans[1] = dx;
    geoTrans[2] = 0;
    geoTrans[3] = yll + nRows * dx;
    geoTrans[4] = 0;
    geoTrans[5] = -dx;
    poDstDS->SetGeoTransform(geoTrans);

    OGRSpatialReference srs;
    srs.SetACEA(25, 47, 0, 105, 0, 0);
    srs.SetWellKnownGeogCS("WGS84");

    char *pSrsWkt = NULL;
    srs.exportToWkt(&pSrsWkt);
    poDstDS->SetProjection(pSrsWkt);
    CPLFree(pSrsWkt);

    GDALClose(poDstDS);
}

//set all elements except NoDataValue to zero
template<typename T>
void Raster<T>::SetZero()
{
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            if (!FloatEqual(m_data[i][j], m_noDataValue))
            {
                m_data[i][j] = 0;
            }
        }
    }
}

template<typename T>
void Raster<T>::SetDefaultValues(T value)
{
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            if (FloatEqual(float(m_data[i][j]), float(m_noDataValue)))
            {
                m_data[i][j] = value;
            }
        }
    }
}

template<typename T>
int Raster<T>::Copy(Raster &otherRaster)
{
    //delete existing memory
    DeleteExistingData();

    m_nRows = otherRaster.GetNumberOfRows();
    m_nCols = otherRaster.GetNumberofColumns();
    m_xllCenter = otherRaster.GetXllCenter();
    m_yllCenter = otherRaster.GetYllCenter();
    m_dx = otherRaster.GetXCellSize();
    m_dy = otherRaster.GetYCellSize();
    m_noDataValue = otherRaster.GetNoDataValue();

    //allocate memory
    m_data = new T *[m_nRows];
    for (int i = 0; i < m_nRows; ++i)
    {
        m_data[i] = new T[m_nCols];
        for (int j = 0; j < m_nCols; ++j)
        {
            m_data[i][j] = otherRaster.At(i, j);
        }
    }

    return 0;
}

template<typename T>
void Raster<T>::Add(Raster &otherRaster)
{
    for (int i = 0; i < m_nRows; ++i)
    {
        for (int j = 0; j < m_nCols; ++j)
        {
            if (FloatEqual(m_data[i][j], m_noDataValue))
                continue;
            m_data[i][j] += otherRaster.m_data[i][j];
        }
    }
}

template<typename T>
void Raster<T>::DeleteExistingData(void)
{
    if (m_data != NULL)
    {
        for (int i = 0; i < m_nRows; ++i)
        {
            if (m_data[i] != NULL)
                delete m_data[i];
        }
        delete m_data;
    }
}

template<typename T>
bool Raster<T>::IsNull(int i, int j)
{
    return abs(m_data[i][j] - m_noDataValue) < 0.0000001;
}

template<typename T>
bool Raster<T>::IsEmpty(void)
{
    return (m_data == NULL);
}

#endif