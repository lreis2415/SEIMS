/*----------------------------------------------------------------------
*	Purpose:  Raster Data
*
*	Created:	Junzhi Liu
*	Date:		29-July-2012
*---------------------------------------------------------------------*/


#include <fstream>
#include <iostream>
#include <string>
#include "GridLayering.h"
#include "MongoUtil.h"

using namespace std;

void ReadArcAscii(const char *filename, RasterHeader &rs, int *&data) {
    ifstream rasterFile(filename);

    string tmp;
    //read header
    rasterFile >> tmp >> rs.nCols;
    rasterFile >> tmp >> rs.nRows;
    rasterFile >> tmp >> rs.xll;
    rasterFile >> tmp >> rs.yll;
    rasterFile >> tmp >> rs.dx;

    rasterFile >> tmp >> rs.noDataValue;

    int n = rs.nRows * rs.nCols;
    data = new int[n];

    int nRows = rs.nRows;
    int nCols = rs.nCols;

    //read file
    for (int i = 0; i < nRows; ++i) {
        for (int j = 0; j < nCols; ++j) {
            rasterFile >> data[i * nCols + j];
        }
    }

    rasterFile.close();

}

void OutputArcAscii(const char *filename, RasterHeader &rs, int *data, int noDataValue) {
    ofstream rasterFile(filename);
    //write header
    rasterFile << "NCOLS " << rs.nCols << "\n";
    rasterFile << "NROWS " << rs.nRows << "\n";
    rasterFile << "XLLCENTER " << rs.xll << "\n";
    rasterFile << "YLLCENTER " << rs.yll << "\n";
    rasterFile << "CELLSIZE " << rs.dx << "\n";
    rasterFile << "NODATA_VALUE " << noDataValue << "\n";

    //write file
    for (int i = 0; i < rs.nRows; ++i) {
        for (int j = 0; j < rs.nCols; ++j) {
            rasterFile << data[i * rs.nCols + j] << "\t";
        }
        rasterFile << "\n";
    }

    rasterFile.close();
}

void ReadFromMongo(mongoc_gridfs_t *gfs, const char *remoteFilename, RasterHeader &rs, int *&data) {
    char *buf;
    size_t length;
    bson_t *bmeta;
    MongoGridFS().getStreamData(string(remoteFilename), buf, length, gfs);
    bmeta = MongoGridFS().getFileMetadata(string(remoteFilename), gfs);
    float *dataMongo = (float *) buf;
    GetNumericFromBson(bmeta, "NCOLS", rs.nCols);
    GetNumericFromBson(bmeta, "NROWS", rs.nRows);
    GetNumericFromBson(bmeta, "NODATA_VALUE", rs.noDataValue);
    GetNumericFromBson(bmeta, "XLLCENTER", rs.xll);
    GetNumericFromBson(bmeta, "YLLCENTER", rs.yll);
    GetNumericFromBson(bmeta, "CELLSIZE", rs.dx);
    bson_destroy(bmeta);
    int n = rs.nRows * rs.nCols;
    data = new int[n];
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        data[i] = (int) dataMongo[i];
    }
    free(buf);
    dataMongo = NULL;
}

void ReadFromMongoFloat(mongoc_gridfs_t *gfs, const char *remoteFilename, RasterHeader &rs, float *&data) {
    char *buf;
    size_t length;
    bson_t *bmeta;
    MongoGridFS().getStreamData(string(remoteFilename), buf, length, gfs);
    bmeta = MongoGridFS().getFileMetadata(string(remoteFilename), gfs);
    GetNumericFromBson(bmeta, "NCOLS", rs.nCols);
    GetNumericFromBson(bmeta, "NROWS", rs.nRows);
    GetNumericFromBson(bmeta, "NODATA_VALUE", rs.noDataValue);
    GetNumericFromBson(bmeta, "XLLCENTER", rs.xll);
    GetNumericFromBson(bmeta, "YLLCENTER", rs.yll);
    GetNumericFromBson(bmeta, "CELLSIZE", rs.dx);
    bson_destroy(bmeta);
    int n = rs.nRows * rs.nCols;
    if (4 * n != length) {
        cout << "The data length of " << remoteFilename << " is not consistent with metadata.\n";
        return;
    }
    data = (float *) buf;
}

bool WriteStringToMongoDB(mongoc_gridfs_t *gfs, int id, const char *type, int number, char *s) {
    bson_t p = BSON_INITIALIZER;
    BSON_APPEND_INT32(&p, "SUBBASIN", id);
    BSON_APPEND_UTF8(&p, "TYPE", type);

    char remoteFilename[100];
    strprintf(remoteFilename, 100, "%d_%s", id, type);
    BSON_APPEND_UTF8(&p, "ID", remoteFilename);
    BSON_APPEND_UTF8(&p, "DESCRIPTION", type);
    BSON_APPEND_DOUBLE(&p, "NUMBER", number);
    MongoGridFS().removeFile(string(remoteFilename), gfs);

    size_t n = number * sizeof(float);
    MongoGridFS().writeStreamData(string(remoteFilename), s, n, &p, gfs);
    bson_destroy(&p);
    if (NULL == MongoGridFS().getFile(remoteFilename, gfs)) {
        return false;
    } 
    else {
        return true;
    }
}

bool OutputLayersToMongoDB(const char *layeringTxtFile, const char *dataType, int id, mongoc_gridfs_t *gfs) {
    ifstream ifs(layeringTxtFile);
    int nValidGrids, nLayers;
    ifs >> nValidGrids >> nLayers;

    int n = nValidGrids + nLayers + 1;
    float *pLayers = new float[n];
    pLayers[0] = nLayers;
    int index = 1;

    for (int i = 1; i < n; ++i) {
        ifs >> pLayers[i];
    }
    ifs.close();
    bool flag = true;
    if (!WriteStringToMongoDB(gfs, id, dataType, n, (char *)pLayers)) {
        flag = false;
    }
    delete[] pLayers;
    pLayers = NULL;
    return flag;
}
