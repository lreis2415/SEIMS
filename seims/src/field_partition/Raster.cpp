/*----------------------------------------------------------------------
*	Purpose: 	Raster Data
*
*	Created:	Junzhi Liu
*	Date:		29-July-2010
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
#pragma once
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


using namespace std;

template <typename T>
inline Raster<T>::Raster(void):m_data(NULL), m_nRows(0), m_nCols(0), 
					 m_xllCenter(0), m_yllCenter(0), m_dx(0), m_dy(0), m_noDataValue(-9999)
{
}

template <typename T>
inline Raster<T>::~Raster(void)
{
	DeleteExistingData();
}

template <typename T>
inline int Raster<T>::ReadFromArcAsc(const char* filename)
{
	ifstream rasterFile(filename);
	string tmp;
	//read header
	rasterFile >> tmp >> m_nCols;
	rasterFile >> tmp >> m_nRows;
	rasterFile >> tmp >> m_xllCenter;
	rasterFile >> tmp >> m_yllCenter;
	rasterFile >> tmp >> m_dx;
	m_dy = m_dx;
	rasterFile >> tmp >> m_noDataValue;
	m_srs = "";
	//allocate memory
	DeleteExistingData();
	m_data = new T*[m_nRows];
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

template <typename T>
int Raster<T>::ReadFromMWAsc(const char* filename)
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
	m_srs = "";
	//allocate memory
	DeleteExistingData();
	m_data = new T*[m_nRows];
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

template <typename T>
int Raster<T>::ReadFromMongoDB(gridfs *gfs, const char* remoteFilename)
{
	gridfile gfile[1];
	bson b[1];
	bson_init(b);
	bson_append_string(b, "filename",  remoteFilename);
	bson_finish(b);  
	int flag = gridfs_find_query(gfs, b, gfile); 
	if(0 != flag)
	{  
		cout << "Failed in ReadFromMongoDB, Remote file: " << remoteFilename << endl;
		return -1;
	}

	size_t length = (size_t)gridfile_get_contentlength(gfile);
	char* buf = (char*)malloc(length);
	gridfile_read (gfile, length, buf);
	float *data = (float*)buf;

	bson bmeta[1];
	gridfile_get_metadata(gfile, bmeta);
	bson_iterator iterator[1];
	if ( bson_find( iterator, bmeta, "NCOLS" )) 
		m_nCols = bson_iterator_int(iterator);
	if ( bson_find( iterator, bmeta, "NROWS" )) 
		m_nRows = bson_iterator_int(iterator);
	if ( bson_find( iterator, bmeta, "NODATA_VALUE" )) 
		m_noDataValue = (T)bson_iterator_double(iterator);
	if ( bson_find( iterator, bmeta, "XLLCENTER" )) 
		m_xllCenter = (float)bson_iterator_double(iterator);
	if ( bson_find( iterator, bmeta, "YLLCENTER" )) 
		m_yllCenter = (float)bson_iterator_double(iterator);
	if ( bson_find( iterator, bmeta, "CELLSIZE" )) 
	{
		m_dx = (float)bson_iterator_double(iterator);
		m_dy = m_dx;
	}
	//bson_destroy(bmeta);

	//allocate memory
	DeleteExistingData();
	m_data = new T*[m_nRows];
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
			index = i*m_nCols + j;
			m_data[i][j] = T(data[index]);
		}
	}

	bson_destroy(b);
	gridfile_destroy(gfile);
	return 0;
}

template <typename T>
int Raster<T>::ReadFromGDAL(const char* filename)
{
	GDALDataset  *poDataset = (GDALDataset *) GDALOpen( filename, GA_ReadOnly );
	if( poDataset == NULL )
	{
		cerr << "Open raster file failed.\n";
		return -1;
	}

	GDALRasterBand  *poBand= poDataset->GetRasterBand(1);
	m_nCols = poBand->GetXSize();
	m_nRows = poBand->GetYSize();
	m_noDataValue = (float)poBand->GetNoDataValue();
	double adfGeoTransform[6];
	poDataset->GetGeoTransform(adfGeoTransform);
	m_dx = adfGeoTransform[1];
	m_dy = -adfGeoTransform[5];
	m_xllCenter = adfGeoTransform[0];
	m_yllCenter = adfGeoTransform[3] - (m_nRows)*m_dy;
	m_srs = string(poDataset->GetProjectionRef());
	//allocate memory
	DeleteExistingData();
	m_data = new T*[m_nRows];
	for (int i = 0; i < m_nRows; ++i)
	{
		m_data[i] = new T[m_nCols];
	}
	GDALDataType dataType = poBand->GetRasterDataType();
	
	if(dataType == GDT_Int32)
	{
		int* pData = (int *) CPLMalloc(sizeof(int)*m_nCols*m_nRows);
		poBand->RasterIO(GF_Read, 0, 0, m_nCols, m_nRows, 
		pData, m_nCols, m_nRows, GDT_Int32, 0, 0);
		for (int i = 0; i < m_nRows; i++)
		{
			for (int j = 0; j < m_nCols; j++)
			{
				int index = i*m_nCols + j;
				m_data[i][j] = T(pData[index]);
			}
		}
	}
	else
	{
		float* pData = (float *) CPLMalloc(sizeof(float)*m_nCols*m_nRows);
		poBand->RasterIO(GF_Read, 0, 0, m_nCols, m_nRows, 
		pData, m_nCols, m_nRows, GDT_Float32, 0, 0);
		for (int i = 0; i < m_nRows; i++)
		{
			for (int j = 0; j < m_nCols; j++)
			{
				int index = i*m_nCols + j;
				m_data[i][j] = T(pData[index]);
			}
		}
	}
	GDALClose(poDataset);

	return 0;
}

template <typename T>
int Raster<T>::OutputMWAscii(const char* filename)
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
template <typename T>
int Raster<T>::OutputGeoTiff(const char* filename)
{
	/// Output GeoTiff all set as float datatype. by LJ.
	int n = m_nRows * m_nCols;
	float *data = new float[n];
	int index = 0;
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			data[i*m_nCols+j] = float(m_data[i][j]);
		}
	}
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	char **papszOptions = poDriver->GetMetadata();
	GDALDataset *poDstDS = poDriver->Create(filename, m_nCols, m_nRows, 1, GDT_Float32, papszOptions );

	/// Write the data to new file
	GDALRasterBand  *poDstBand= poDstDS->GetRasterBand(1);
	poDstBand->RasterIO(GF_Write, 0, 0,  m_nCols, m_nRows, data,  m_nCols, m_nRows, GDT_Float32, 0, 0);
	poDstBand->SetNoDataValue(m_noDataValue);

	double geoTrans[6];
	geoTrans[0] = m_xllCenter;
	geoTrans[1] = m_dx;
	geoTrans[2] = 0;
	geoTrans[3] = m_yllCenter + m_nRows*m_dx;
	geoTrans[4] = 0;
	geoTrans[5] = -m_dx;
	poDstDS->SetGeoTransform(geoTrans);
	poDstDS->SetProjection(m_srs.c_str());
	GDALClose(poDstDS);
	delete[] data;
	return 0;
}
template <typename T>
int Raster<T>::OutputArcAscii(const char* filename)
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

//set all elements except NoDataValue to zero
template <typename T>
void Raster<T>::SetZero()
{
	for (int i = 0; i < m_nRows; ++i)
	{
		for (int j = 0; j < m_nCols; ++j)
		{
			if (!FloatEqual(m_data[i][j] ,m_noDataValue))
			{
				m_data[i][j] = 0;
			}
		}
	}
}

template <typename T>
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

template <typename T>
int Raster<T>::Copy(Raster& otherRaster)
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
	m_srs = otherRaster.GetSRS();
	//allocate memory
	m_data = new T*[m_nRows];
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

template <typename T>
void Raster<T>::Add(Raster& otherRaster)
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

template <typename T>
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

template <typename T>
bool Raster<T>::IsNull(int i, int j)
{
	return abs(m_data[i][j] - m_noDataValue) < UTIL_ZERO;
}

template <typename T>
bool Raster<T>::IsEmpty(void)
{
	return (m_data == NULL);
}
