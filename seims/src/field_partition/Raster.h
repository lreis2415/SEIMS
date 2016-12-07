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
#include "gridfs.h"
#include <string>
using namespace std;
template <typename T>
class Raster
{
public:
	Raster(void);
	~Raster(void);
	int ReadFromArcAsc(const char* filename);
	int ReadFromMWAsc(const char* filename);
	int ReadFromGDAL(const char* filename);
	int ReadFromMongoDB(gridfs *gfs, const char* remoteFilename);
	int Copy(Raster& otherRaster);
	
	int OutputMWAscii(const char* filename);
	int OutputArcAscii(const char* filename);
	int OutputGeoTiff(const char* filename);
	void SetZero();
	void SetDefaultValues(T value);
	void Add(Raster& otherRaster);

	T At(int i, int j) { return m_data[i][j]; }
	void SetValue(int i, int j, T val) { m_data[i][j] = val; }
	int GetNumberOfRows() { return m_nRows; }
	int GetNumberofColumns() { return m_nCols; }
	double GetXllCenter() { return m_xllCenter; }
	double GetYllCenter() { return m_yllCenter; }
	double GetXCellSize() { return m_dx; }
	double GetYCellSize() { return m_dy; }
	T GetNoDataValue() { return m_noDataValue; }
	T** GetData() { return m_data; }
	string GetSRS() {return m_srs;}

private:
	T **m_data;
	T m_noDataValue;
	int m_nRows, m_nCols;
	double m_xllCenter, m_yllCenter;
	double m_dx, m_dy;
	/// OGRSpatialReference
	string m_srs;

private:
	void DeleteExistingData(void);
public:
	bool IsNull(int i, int j);
	bool IsEmpty(void);
};