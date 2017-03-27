#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "utilities.h"
#include "MongoUtil.h"
#include "clsRasterData.cpp"
#include "SubBasin.h"

using namespace std;

int FindBoundingBox(clsRasterData<int> &rsSubbasin, map<int, SubBasin> &bboxMap) {
    int nXSize = rsSubbasin.getCols();
    int nYSize = rsSubbasin.getRows();

    int *pData = rsSubbasin.getRasterDataPointer();
    int noDataValue = rsSubbasin.getNoDataValue();
    // find bounding box for each subbasin
    for (int i = 0; i < nYSize; i++) {
        for (int j = 0; j < nXSize; j++) {
            int id = pData[i * nXSize + j];
            if (noDataValue != id) {
                if (bboxMap.find(id) == bboxMap.end()) {
                    bboxMap[id] = SubBasin(j, i, j, i);
                } else {
                    if (j < bboxMap[id].xMin) {
                        bboxMap[id].xMin = j;
                    } else if (j > bboxMap[id].xMax) {
                        bboxMap[id].xMax = j;
                    }

                    if (i > bboxMap[id].yMax) {
                        bboxMap[id].yMax = i;
                    }
                }
                bboxMap[id].cellCount += 1;
            }
        }
    }
    return 0;
}

int DecompositeRasterToMongoDB(map<int, SubBasin> &bboxMap, clsRasterData<int> &rsSubbasin, const char *dstFile,
                               mongoc_client_t *conn, mongoc_gridfs_t *gfs) {
    clsRasterData<float> rs;
    rs.ReadByGDAL(dstFile, false);

    int nXSize = rs.getCols();
    // int nYSize = rs.getRows();
    float noDataValue = rs.getNoDataValue();
    const char *srs = rs.getSRS();
    //cout << nXSize << "\t" << nYSize << endl;
    float *rsData = rs.getRasterDataPointer();
    int *subbasinData = rsSubbasin.getRasterDataPointer();

    map<int, SubBasin>::iterator it;
    string coreName = GetCoreFileName(dstFile);

    for (it = bboxMap.begin(); it != bboxMap.end(); it++) {
        int id = it->first;
        int subbasinID = id;
        if (bboxMap.size() == 1) {
            id = 0;
        }
        SubBasin &subbasin = it->second;
        int subXSize = subbasin.xMax - subbasin.xMin + 1;
        int subYSize = subbasin.yMax - subbasin.yMin + 1;

        float *subData = new float[subXSize * subYSize];
#pragma omp parallel for
        for (int i = subbasin.yMin; i <= subbasin.yMax; i++) /// row
        {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++) /// col
            {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[index] == subbasinID) {
                    subData[subIndex] = rsData[index];
                } else {
                    subData[subIndex] = noDataValue;
                }
            }
        }

        ostringstream remoteFilename;
        remoteFilename << id << "_" << GetUpper(coreName);
        float cellSize = rs.getCellWidth();
        MongoGridFS().removeFile(remoteFilename.str(), gfs);

        bson_t p = BSON_INITIALIZER;
        BSON_APPEND_INT32(&p, "SUBBASIN", id);
        BSON_APPEND_UTF8(&p, "TYPE", coreName.c_str());
        BSON_APPEND_UTF8(&p, "ID", remoteFilename.str().c_str());
        BSON_APPEND_UTF8(&p, "DESCRIPTION", coreName.c_str());
        BSON_APPEND_DOUBLE(&p, "CELLSIZE", cellSize);
        BSON_APPEND_DOUBLE(&p, "NODATA_VALUE", rs.getNoDataValue());
        BSON_APPEND_DOUBLE(&p, "NCOLS", subXSize);
        BSON_APPEND_DOUBLE(&p, "NROWS", subYSize);
        BSON_APPEND_DOUBLE(&p, "XLLCENTER", rs.getXllCenter() + subbasin.xMin * cellSize);
        BSON_APPEND_DOUBLE(&p, "YLLCENTER", rs.getYllCenter() + (rs.getRows() - subbasin.yMax - 1) * cellSize);
        BSON_APPEND_DOUBLE(&p, "LAYERS", rs.getLayers());
        BSON_APPEND_DOUBLE(&p, "CELLSNUM", rs.getCellNumber());
        BSON_APPEND_UTF8(&p, "SRS", srs);

        char *databuf = (char *) subData;
        size_t datalength = sizeof(float) * subXSize * subYSize;
        MongoGridFS().writeStreamData(remoteFilename.str(), databuf, datalength, &p, gfs);
        bson_destroy(&p);

        databuf = NULL;
        Release1DArray(subData);
    }
    return 0;
}

int Decomposite2DRasterToMongoDB(map<int, SubBasin> &bboxMap, clsRasterData<int> &rsSubbasin, string coreName,
                                 vector <string> dstFiles, mongoc_client_t *conn, mongoc_gridfs_t *gfs) {
    int colNum = dstFiles.size();
    clsRasterData<float> rss(dstFiles, false);
    int nXSize = rss.getCols();
    // int nYSize = rss.getRows();
    float noDataValue = rss.getNoDataValue();
    const char *srs = rss.getSRS();
    ///cout << nXSize << "\t" << nYSize << endl;
    float **rssData = rss.get2DRasterDataPointer();
    int *subbasinData = rsSubbasin.getRasterDataPointer();
    map<int, SubBasin>::iterator it;
    for (it = bboxMap.begin(); it != bboxMap.end(); it++) {
        int id = it->first;
        int subbasinID = id;
        if (bboxMap.size() == 1) {
            id = 0;
        }
        SubBasin &subbasin = it->second;
        int subXSize = subbasin.xMax - subbasin.xMin + 1;
        int subYSize = subbasin.yMax - subbasin.yMin + 1;
        int subCellNum = subXSize * subYSize;
        float *sub2DData = NULL;
        Initialize1DArray(subCellNum * colNum, sub2DData, noDataValue);
#pragma omp parallel for
        for (int i = subbasin.yMin; i <= subbasin.yMax; i++) {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++) {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[index] == subbasinID) {
                    for (int k = 0; k < colNum; k++) {
                        sub2DData[subIndex * colNum + k] = rssData[index][k];
                    }
                }
            }
        }
        ostringstream remoteFilename;
        remoteFilename << id << "_" << GetUpper(coreName);
        float cellSize = rss.getCellWidth();
        MongoGridFS().removeFile(remoteFilename.str(), gfs);

        bson_t p = BSON_INITIALIZER;
        BSON_APPEND_INT32(&p, "SUBBASIN", id);
        BSON_APPEND_UTF8(&p, "TYPE", coreName.c_str());
        BSON_APPEND_UTF8(&p, "ID", remoteFilename.str().c_str());
        BSON_APPEND_UTF8(&p, "DESCRIPTION", coreName.c_str());
        BSON_APPEND_DOUBLE(&p, "CELLSIZE", cellSize);
        BSON_APPEND_DOUBLE(&p, "NODATA_VALUE", rss.getNoDataValue());
        BSON_APPEND_DOUBLE(&p, "NCOLS", subXSize);
        BSON_APPEND_DOUBLE(&p, "NROWS", subYSize);
        BSON_APPEND_DOUBLE(&p, "XLLCENTER", rss.getXllCenter() + subbasin.xMin * cellSize);
        BSON_APPEND_DOUBLE(&p, "YLLCENTER", rss.getYllCenter() + (rss.getRows() - subbasin.yMax - 1) * cellSize);
        BSON_APPEND_DOUBLE(&p, "LAYERS", rss.getLayers());
        BSON_APPEND_DOUBLE(&p, "CELLSNUM", rss.getCellNumber());
        BSON_APPEND_UTF8(&p, "SRS", srs);

        char *databuf = (char *) sub2DData;
        size_t datalength = sizeof(float) * subCellNum * colNum;
        MongoGridFS().writeStreamData(remoteFilename.str(), databuf, datalength, &p, gfs);
        bson_destroy(&p);
        databuf = NULL;
        Release1DArray(sub2DData);
    }
    srs = NULL;
    rssData = NULL;
    subbasinData = NULL;
    return 0;
}

int DecompositeRaster(map<int, SubBasin> &bboxMap, clsRasterData<int> &rsSubbasin, const char *dstFile,
                      const char *tmpFolder) {
    clsRasterData<float> rs;
    rs.ReadByGDAL(dstFile, false);

    int nXSize = rs.getCols();
    // int nYSize = rs.getRows();
    float noDataValue = rs.getNoDataValue();

    float *rsData = rs.getRasterDataPointer();
    int *subbasinData = rsSubbasin.getRasterDataPointer();

    const char *pszFormat = "GTiff";
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    map<int, SubBasin>::iterator it;
    string coreName = GetCoreFileName(dstFile);
    for (it = bboxMap.begin(); it != bboxMap.end(); it++) {
        int id = it->first;
        int subbasinID = id;
        if (bboxMap.size() == 1) { // means not for Cluster
            id = 0;
        }
        SubBasin &subbasin = it->second;
        int subXSize = subbasin.xMax - subbasin.xMin + 1;
        int subYSize = subbasin.yMax - subbasin.yMin + 1;

        ostringstream oss;
        oss << tmpFolder << "/" << id << "/" << GetUpper(coreName) << ".tif";
        string subbasinFile = oss.str();
        //cout << subbasinFile << endl;
        // create a new raster for the subbasin
        char **papszOptions = NULL;
        GDALDataset *poDstDS = poDriver->Create(subbasinFile.c_str(), subXSize, subYSize, 1, GDT_Float32, papszOptions);

        float *subData = (float *) CPLMalloc(sizeof(float) * subXSize * subYSize);
#pragma omp parallel for
        for (int i = subbasin.yMin; i <= subbasin.yMax; i++) {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++) {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[index] == subbasinID) {
                    subData[subIndex] = rsData[index];
                } else {
                    subData[subIndex] = noDataValue;
                }
            }
        }

        //write the data to new file
        GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
        poDstBand->RasterIO(GF_Write, 0, 0, subXSize, subYSize,
                            subData, subXSize, subYSize, GDT_Float32, 0, 0);
        poDstBand->SetNoDataValue(noDataValue);

        double geoTrans[6];
        float cellSize = rs.getCellWidth();
        geoTrans[0] = rs.getXllCenter() + (subbasin.xMin - 0.5f) * cellSize;
        geoTrans[1] = cellSize;
        geoTrans[2] = 0;
        geoTrans[3] = rs.getYllCenter() + (rs.getRows() - subbasin.yMin - 0.5f) * cellSize;
        geoTrans[4] = 0;
        geoTrans[5] = -cellSize;
        poDstDS->SetGeoTransform(geoTrans);

        CPLFree(subData);
        GDALClose(poDstDS);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 7) {
        cout << "Usage: " <<
             "ImportRaster <MaskFile> <DataFolder> <modelName> <GridFSName> <hostIP> <port> [outputFolder]\n";
        exit(-1);
    }

    GDALAllRegister();
    /// set default OpenMP thread number to improve compute efficiency
    SetDefaultOpenMPThread();

    //const char* subbasinFile = "F:\\modeldev\\integrated\\storm_model\\Debug\\model_lyg_10m\\mask.asc";
    //const char* folder = "F:\\modeldev\\integrated\\storm_model\\Debug\\model_lyg_10m";
    //const char* modelName = "model_lyg_10m";
    //const char* hostname = "192.168.5.195";
    const char *subbasinFile = argv[1];
    const char *folder = argv[2];
    const char *modelName = argv[3];
    const char *gridFSName = argv[4];
    const char *hostname = argv[5];
    int port = atoi(argv[6]);
    const char *outTifFolder = NULL;

    if (argc >= 8) {
        outTifFolder = argv[7];
    }
    //cout<<argc<<endl;
    //for(int i = 0; i < argc; i++)
    //	cout<<argv[i]<<endl;
    vector <string> dstFiles;
    FindFiles(folder, "*.tif", dstFiles);
    cout << "File number:" << dstFiles.size() << endl;
    //for (vector<string>::iterator it = dstFiles.begin(); it != dstFiles.end(); it++)
    //{
    //	cout<<*it<<",";
    //}
    /// Identify Array1D and Array2D dstFiles, respectively
    vector <string> coreFileNames;
    vector <string> array1DFiles;
    map <string, vector<string>> array2DFiles;
    map < string, vector < string > > ::iterator
    array2DIter;
    for (vector<string>::iterator it = dstFiles.begin(); it != dstFiles.end(); it++) {
        string tmpCoreName = GetCoreFileName(*it);
        coreFileNames.push_back(tmpCoreName);
        vector <string> tokens = SplitString(tmpCoreName, '_');
        int length = tokens.size();

        if (length <= 1) {
            array1DFiles.push_back(*it);
        } else if (length >= 2) /// there are more than one underscore exist
        {
            string::size_type end = tmpCoreName.find_last_of("_");
            string coreVarName = tmpCoreName.substr(0, end);

            if (atoi(tokens[length - 1].c_str())) {
                array2DIter = array2DFiles.find(coreVarName);
                if (array2DIter != array2DFiles.end()) {
                    array2DFiles[coreVarName].push_back(*it);
                } else {
                    vector <string> tmpFileName;
                    tmpFileName.push_back(*it);
                    array2DFiles.insert(make_pair(coreVarName, tmpFileName));
                }
            } else {
                array1DFiles.push_back(*it);
            }
        }
    }
    vector <string> delVarNames;
    for (array2DIter = array2DFiles.begin(); array2DIter != array2DFiles.end(); array2DIter++) {
        if (array2DIter->second.size() == 1) {
            array1DFiles.push_back(array2DIter->second.at(0));
            delVarNames.push_back(array2DIter->first);
        } else {
            sort(array2DIter->second.begin(), array2DIter->second.end());
        }
    }
    for (vector<string>::iterator it = delVarNames.begin(); it != delVarNames.end(); it++) {
        array2DFiles.erase(*it);
    }
    vector<string>(array1DFiles).swap(array1DFiles);

    //////////////////////////////////////////////////////////////////////////
    // read the subbasin file, and find the bounding box of each subbasin
    clsRasterData<int> rsSubbasin(subbasinFile, false);
    map<int, SubBasin> bboxMap;
    FindBoundingBox(rsSubbasin, bboxMap);

    //////////////////////////////////////////////////////////////////////////
    /// loop to process the destination files

    /// connect to MongoDB
    MongoClient client = MongoClient(hostname, port);
    mongoc_client_t *conn = client.getConn();
    mongoc_gridfs_t *gfs = client.getGridFS(string(modelName), string(gridFSName));

    cout << "Importing spatial data to MongoDB...\n";
    for (array2DIter = array2DFiles.begin(); array2DIter != array2DFiles.end(); array2DIter++) {
        vector <string> tmpFileNames = array2DIter->second;
        for (vector<string>::iterator it = tmpFileNames.begin(); it != tmpFileNames.end(); it++) {
            cout << "\t" << *it << endl;
            if (outTifFolder != NULL) {
                DecompositeRaster(bboxMap, rsSubbasin, it->c_str(), outTifFolder);
            }
        }
        Decomposite2DRasterToMongoDB(bboxMap, rsSubbasin, array2DIter->first, tmpFileNames, conn, gfs);
    }
    for (size_t i = 0; i < array1DFiles.size(); ++i) {
        cout << "\t" << array1DFiles[i] << endl;
        if (outTifFolder != NULL) {
            DecompositeRaster(bboxMap, rsSubbasin, array1DFiles[i].c_str(), outTifFolder);
        }
        DecompositeRasterToMongoDB(bboxMap, rsSubbasin, array1DFiles[i].c_str(), conn, gfs);
    }
    mongoc_gridfs_destroy(gfs);
}
