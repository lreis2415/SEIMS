#include <sstream>
//gdal
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"

#include "util.h"
#include "parallel.h"
#include "clsRasterData.cpp"

//#include "Raster.h"
using namespace std;
void CombineRasters(map<int, clsRasterData<float>* > *allRasterData, string &m_srs, string &outputRaster)
{
	float xMin = FLT_MAX;
	float xMax = 0.f;
	float yMin = FLT_MAX;
	float yMax = 0.f;

	float xll = 0.f, yll = 0.f, dx = 0.f, dy = 0.f, xur = 0.f, yur = 0.f;
	int nRows = 0, nCols = 0;
	int nSubbasins = allRasterData->size();
	// loop to get global extent
	for (int i = 1; i <= nSubbasins; i++)
	{
		clsRasterData<float> *rs = allRasterData->at(i);
		nRows = rs->getRows();
		nCols = rs->getCols();
		dx = rs->getCellWidth();
		dy = dx;
		xll = rs->getXllCenter() - 0.5f * dx;
		yll = rs->getYllCenter() - 0.5f * dy;
		xur = xll + nCols * dx;
		yur = yll + nRows * dy;
		//cout << "[DEBUG]\t" << nRows << "\t" << nCols << "\t" << xll << "\t" << yll << endl;

		if (i == 1)
		{
			xMin = xll;
			yMin = yll;
			xMax = xur;
			yMax = yur;
			continue;
		}

		xMin = (xll < xMin) ? xll : xMin;
		xMax = (xur > xMax) ? xur : xMax;
		yMin = (yll < yMin) ? yll : yMin;
		yMax = (yur > yMax) ? yur : yMax;

	}

	int nColsTotal = int((xMax - xMin) / dx + 0.5);
	int nRowsTotal = int((yMax - yMin) / dy + 0.5);

	//cout << xMin << "\t" << yMax << endl;
	//cout << nRowsTotal << "\t" << nColsTotal << endl;
	int nTotal = nRowsTotal * nColsTotal;
	float *data = new float[nTotal];
	float noDataValue = -9999.f;
	for (int i = 0; i < nTotal; i++)
	{
		data[i] = noDataValue;
	}
	/// create header for the entire raster file
	map<string, double> m_headers;
	m_headers.at(HEADER_RS_NCOLS) = (double) nColsTotal;
	m_headers.at(HEADER_RS_NCOLS) = (double) nColsTotal;
	m_headers.at(HEADER_RS_NROWS) = (double) nRowsTotal;
	m_headers.at(HEADER_RS_NODATA) = (double) noDataValue;
	m_headers.at(HEADER_RS_CELLSIZE) = (double) dx;
	m_headers.at(HEADER_RS_XLL) = (double) xMin;
	m_headers.at(HEADER_RS_YLL) = (double) yMin;

	// loop to put data in the array
	for (int i = 1; i <= nSubbasins; i++)
	{
		clsRasterData<float> *rs = allRasterData->at(i);
		nRows = rs->getRows();
		nCols = rs->getCols();
		dx = rs->getCellWidth();
		dy = dx;
		xll = rs->getXllCenter() - 0.5f * dx;
		yll = rs->getYllCenter() - 0.5f * dy;
		yur = yll + nRows * dy;
		float subNoDataValue = rs->getNoDataValue(); // use this or -9999?
		int cellNums = 0;
		float *subData = NULL; /// all geotif should be 1D data
		float **validPosition = NULL;
		rs->getRasterData(&cellNums, &subData);
		rs->getRasterPositionData(&cellNums, &validPosition);
		for (int idx = 0; idx < cellNums; idx++)
		{
			int k = validPosition[idx][0];
			int m = validPosition[idx][1];
			int i = int((yMax - yur + (k + 0.5) * dy) / dy); /// row in whole extent
			int j = int((xll + (m + 0.5) * dx - xMin) / dx); /// col in whole extent

			int index = i * nColsTotal + j; /// index in whole extent
			data[index] = subData[idx];
		}
	}
	clsRasterData<float>::outputGTiff(m_headers, m_srs, data, outputRaster);
	cout << "[DEBUG]\toutput file: " << outputRaster << endl;
}

void CombineRasterResults(string &folder, string &sVar, string &fileType, int nSubbasins, string &outputRaster)
{
	string m_srs;
	map<int, clsRasterData<float>* > allRasterData;
	for (int i = 1; i <= nSubbasins; i++){
		string curFileName = ValueToString(i) + "_" + sVar + fileType;
		clsRasterData<float> rs;
		
		if (string(ASCIIExtension).find(fileType) != string(ASCIIExtension).npos) /// ASC file
			rs.ReadASCFile(curFileName.c_str());
		else /// GTiff file
			rs.ReadFromGDAL(curFileName.c_str());
		if (m_srs == "")
			m_srs = rs.getSRSString();
		allRasterData[i] = &rs;
	}
	CombineRasters(&allRasterData, m_srs, outputRaster);
 //   float xMin = FLT_MAX; /// previous code is 1.e10
 //   float xMax = 0.f;
 //   float yMin = FLT_MAX;
 //   float yMax = 0.f;

 //   float xll, yll, dx, dy, nRows, nCols, xur, yur;
	//string m_srs = "";
 //   // ostringstream oss;
	//// read and store all raster data
	//map<int, clsRasterData<float>* > allRasterData;
 //   // loop to get global extent
 //   for (int i = 1; i <= nSubbasins; i++)
 //   {
 //       //oss.str("");
 //       //oss << folder << i << "_" << sVar << ".asc";
 // //      Raster<float> rs;
	//	//rs.ReadArcAscHeader(oss.str().c_str());
	//	string curFileName = ValueToString(i) + "_" + sVar + fileType;
	//	clsRasterData<float> rs;
	//	
	//	if (string(ASCIIExtension).find(fileType) != string(ASCIIExtension).npos) /// ASC file
	//		rs.ReadASCFile(curFileName.c_str());
	//	else /// GTiff file
	//		rs.ReadFromGDAL(curFileName.c_str());
	//	if (m_srs == "")
	//		m_srs = rs.getSRSString();
	//	allRasterData[i] = &rs;
 //       //nRows = rs.GetNumberOfRows();
 //       //nCols = rs.GetNumberofColumns();
 //       //dx = rs.GetXCellSize();
 //       //dy = dx;
 //       //xll = rs.GetXllCenter() - 0.5f * dx;
 //       //yll = rs.GetYllCenter() - 0.5f * dy;
 //       //xur = xll + nCols * dx;
 //       //yur = yll + nRows * dy;

	//	nRows = rs.getRows();
	//	nCols = rs.getCols();
	//	dx = rs.getCellWidth();
	//	dy = dx;
	//	xll = rs.getXllCenter() - 0.5f * dx;
	//	yll = rs.getYllCenter() - 0.5f * dy;
	//	xur = xll + nCols * dx;
	//	yur = yll + nRows * dy;
 //       //cout << "[DEBUG]\t" << nRows << "\t" << nCols << "\t" << xll << "\t" << yll << endl;

 //       if (i == 1)
 //       {
 //           xMin = xll;
 //           yMin = yll;
 //           xMax = xur;
 //           yMax = yur;
 //           continue;
 //       }

 //       xMin = (xll < xMin) ? xll : xMin;
 //       xMax = (xur > xMax) ? xur : xMax;
 //       yMin = (yll < yMin) ? yll : yMin;
 //       yMax = (yur > yMax) ? yur : yMax;

 //   }

 //   int nColsTotal = int((xMax - xMin) / dx + 0.5);
 //   int nRowsTotal = int((yMax - yMin) / dy + 0.5);

 //   //cout << xMin << "\t" << yMax << endl;
 //   //cout << nRowsTotal << "\t" << nColsTotal << endl;
 //   int nTotal = nRowsTotal * nColsTotal;
 //   float *data = new float[nTotal];
 //   float noDataValue = -9999.f;
	//for (int i = 0; i < nTotal; i++)
	//{
	//	data[i] = noDataValue;
	//}
	///// create header for the entire raster file
	//map<string, double> *m_headers;
	//m_headers->at(HEADER_RS_NCOLS) = (double) nColsTotal;
	//m_headers->at(HEADER_RS_NCOLS) = (double) nColsTotal;
	//m_headers->at(HEADER_RS_NROWS) = (double) nRowsTotal;
	//m_headers->at(HEADER_RS_NODATA) = (double) noDataValue;
	//m_headers->at(HEADER_RS_CELLSIZE) = (double) dx;
	//m_headers->at(HEADER_RS_XLL) = (double) xMin;
	//m_headers->at(HEADER_RS_YLL) = (double) yMin;
	//
 //   // loop to put data in the array
 //   for (int i = 1; i <= nSubbasins; i++)
 //   {
 //       //cout << i << endl;
 //       //oss.str("");
 //       //oss << folder << i << "_" << sVar << ".asc";
 //       //Raster<float> rs;
 //       //rs.ReadFromArcAsc(oss.str().c_str());

 //       //nRows = rs.GetNumberOfRows();
 //       //nCols = rs.GetNumberofColumns();
 //       //dx = rs.GetXCellSize();
 //       //dy = dx;
 //       //xll = rs.GetXllCenter() - 0.5f * dx;
 //       //yll = rs.GetYllCenter() - 0.5f * dy;
 //       //yur = yll + nRows * dy;
 //       //float subNoDataValue = rs.GetNoDataValue();
 //       //float **subData = rs.GetData();

	//	clsRasterData<float>* rs = allRasterData[i];
	//	nRows = rs->getRows();
	//	nCols = rs->getCols();
	//	dx = rs->getCellWidth();
	//	dy = dx;
	//	xll = rs->getXllCenter() - 0.5f * dx;
	//	yll = rs->getYllCenter() - 0.5f * dy;
	//	yur = yll + nRows * dy;
	//	float subNoDataValue = rs->getNoDataValue();
	//	int cellNums = 0;
	//	float *subData = NULL; /// all geotif should be 1D data
	//	float **validPosition = NULL;
	//	rs->getRasterData(&cellNums, &subData);
	//	rs->getRasterPositionData(&cellNums, &validPosition);
	//	for (int idx = 0; idx < cellNums; idx++)
	//	{
	//		int k = validPosition[idx][0];
	//		int m = validPosition[idx][1];
	//		int i = int((yMax - yur + (k + 0.5) * dy) / dy); /// row in whole extent
	//		int j = int((xll + (m + 0.5) * dx - xMin) / dx); /// col in whole extent

	//		int index = i * nColsTotal + j; /// index in whole extent
	//		data[index] = subData[idx];
	//	}
 //       //for (int k = 0; k < nRows; k++)
 //       //{
 //       //    for (int m = 0; m < nCols; m++)
 //       //    {
 //       //        if (FloatEqual(subData[k][m], subNoDataValue))
 //       //            continue;

 //       //        int i = int((yMax - yur + (k + 0.5) * dy) / dy);
 //       //        int j = int((xll + (m + 0.5) * dx - xMin) / dx);

 //       //        int index = i * nColsTotal + j;
 //       //        data[index] = subData[k][m];
 //       //    }
 //       //}
 //   }

 //   // Raster<float>::OutputGTiff(outputRaster.c_str(), nRowsTotal, nColsTotal, xMin, yMin, dx, noDataValue, data);
	//clsRasterData<float>::outputGTiff(m_headers, m_srs, data, outputRaster);
 //   cout << "[DEBUG]\toutput file: " << outputRaster << endl;

}
void CombineRasterResultsMongo(mongoc_gridfs_t *gfs, string &sVar, int nSubbasins, string &outputRaster)
{
	string m_srs;
	map<int, clsRasterData<float>* > allRasterData;
	for (int i = 1; i <= nSubbasins; i++){
		string curFileName = ValueToString(i) + "_" + sVar;
		clsRasterData<float> rs;
		rs.ReadFromMongoDB(gfs, curFileName.c_str());
		if (m_srs == "")
			m_srs = rs.getSRSString();
		allRasterData[i] = &rs;
	}
	CombineRasters(&allRasterData, m_srs, outputRaster);
}

//void CombineRasterResultsMongo(mongoc_gridfs_t *gfs, string &sVar, int nSubbasins, string &outputRaster)
//{
//    float xMin = 1e10;
//    float xMax = 0.f;
//    float yMin = 1e10;
//    float yMax = 0.f;
//
//    float xll, yll, dx, dy, nRows, nCols, xur, yur;
//    // loop to get global extent
//    ostringstream oss;
//
//    for (int i = 1; i <= nSubbasins; i++)
//    {
//        oss.str("");
//        oss << i << "_" << sVar;
//
//        Raster<float> rs;
//        rs.ReadMongoDBHeader(gfs, oss.str().c_str());
//
//        nRows = rs.GetNumberOfRows();
//        nCols = rs.GetNumberofColumns();
//        dx = rs.GetXCellSize();
//        dy = dx;
//        xll = rs.GetXllCenter() - 0.5f * dx;
//        yll = rs.GetYllCenter() - 0.5f * dy;
//        xur = xll + nCols * dx;
//        yur = yll + nRows * dy;
//        //cout << "[DEBUG]\t" << nRows << "\t" << nCols << "\t" << xll << "\t" << yll << endl;
//
//        if (i == 1)
//        {
//            xMin = xll;
//            yMin = yll;
//            xMax = xur;
//            yMax = yur;
//            continue;
//        }
//
//        xMin = (xll < xMin) ? xll : xMin;
//        xMax = (xur > xMax) ? xur : xMax;
//        yMin = (yll < yMin) ? yll : yMin;
//        yMax = (yur > yMax) ? yur : yMax;
//
//    }
//
//    int nColsTotal = int((xMax - xMin) / dx + 0.5);
//    int nRowsTotal = int((yMax - yMin) / dy + 0.5);
//
//    //cout << xMin << "\t" << yMax << endl;
//    //cout << nRowsTotal << "\t" << nColsTotal << endl;
//
//    int nTotal = nRowsTotal * nColsTotal;
//    float *data = new float[nTotal];
//    float noDataValue = -9999;
//    for (int i = 0; i < nTotal; i++)
//    {
//        data[i] = noDataValue;
//    }
//
//    // loop to put data in the array
//    for (int i = 1; i <= nSubbasins; i++)
//    {
//        //cout << i << endl;
//        oss.str("");
//        oss << i << "_" << sVar;
//        Raster<float> rs;
//        rs.ReadFromMongoDB(gfs, oss.str().c_str());
//
//        nRows = rs.GetNumberOfRows();
//        nCols = rs.GetNumberofColumns();
//        dx = rs.GetXCellSize();
//        dy = dx;
//        xll = rs.GetXllCenter() - 0.5f * dx;
//        yll = rs.GetYllCenter() - 0.5f * dy;
//        yur = yll + nRows * dy;
//        float subNoDataValue = rs.GetNoDataValue();
//        float **subData = rs.GetData();
//
//        for (int k = 0; k < nRows; k++)
//        {
//            for (int m = 0; m < nCols; m++)
//            {
//                if (FloatEqual(subData[k][m], subNoDataValue))
//                    continue;
//
//                int i = int((yMax - yur + (k + 0.5) * dy) / dy);
//                int j = int((xll + (m + 0.5) * dx - xMin) / dx);
//
//                int index = i * nColsTotal + j;
//                data[index] = subData[k][m];
//            }
//        }
//    }
//
//
//    //Raster<float>::OutputRaster(outputFile.c_str(), nRowsTotal, nColsTotal, xMin, yMin, dx, noDataValue, data);
//    Raster<float>::OutputGTiff(outputRaster.c_str(), nRowsTotal, nColsTotal, xMin, yMin, dx, noDataValue, data);
//    cout << "[DEBUG]\toutput file: " << outputRaster << endl;
//
//}

//int main()
//{
//	int nSubbasins = 899;
//	string folder = "D:\\watershed_simulation\\output\\";
//	string sVar = "SURU";
//	GDALAllRegister();
//	CombineRaster(folder, sVar, nSubbasins);
//	return 0;
//}
