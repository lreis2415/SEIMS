/*!
 * \brief Import raster data (full size data include nodata) to MongoDB as GridFS.
 * \change  17-07-02 lj - keep import raster to MongoDB if fails and the max. loop is set to 3.
 */
#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#include "SubBasin.h"

#include "utilities.h"
#include "MongoUtil.h"
#include "clsRasterData.h"

using namespace std;

int FindBoundingBox(clsRasterData<int>* rsSubbasin, map<int, SubBasin> &bboxMap) {
    int nXSize = rsSubbasin->getCols();
    int nYSize = rsSubbasin->getRows();

    int *pData = rsSubbasin->getRasterDataPointer();
    int noDataValue = rsSubbasin->getNoDataValue();
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

/*!
 * \brief Decomposite single layer raster data as 1D array, and import to MongoDB as GridFS
 * \param[in] bboxMap Map of subbasin extent box, key is subbasin id, value is \sa SubBasin object
 * \param[in] rsSubbasin Subbasin raster object, \sa clsRasterData
 * \param[in] dstFile Input raster full file path
 * \param[in] conn MongoDB client object
 * \param[in] gfs MongoDB GridFS object
 * \return True if import successfully, otherwise return false.
 */
bool DecompositeRasterToMongoDB(map<int, SubBasin> &bboxMap, clsRasterData<int> *rsSubbasin, const char *dstFile,
                                mongoc_client_t *conn, mongoc_gridfs_t *gfs) {
    bool flag = true;
    clsRasterData<float> *rs = clsRasterData<float>::Init(dstFile, false); // Do not calculate positions data
    if (nullptr == rs) exit(-1);
    int nXSize = rs->getCols();
    // int nYSize = rs.getRows();
    float noDataValue = rs->getNoDataValue();
    const char *srs = rs->getSRS();
    //cout << nXSize << "\t" << nYSize << endl;
    float *rsData = rs->getRasterDataPointer();
    int *subbasinData = rsSubbasin->getRasterDataPointer();

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
        float cellSize = rs->getCellWidth();
        MongoGridFS().removeFile(remoteFilename.str(), gfs);

        bson_t p = BSON_INITIALIZER;
        BSON_APPEND_INT32(&p, "SUBBASIN", id);
        BSON_APPEND_UTF8(&p, "TYPE", coreName.c_str());
        BSON_APPEND_UTF8(&p, "ID", remoteFilename.str().c_str());
        BSON_APPEND_UTF8(&p, "DESCRIPTION", coreName.c_str());
        BSON_APPEND_DOUBLE(&p, "CELLSIZE", cellSize);
        BSON_APPEND_DOUBLE(&p, "NODATA_VALUE", rs->getNoDataValue());
        BSON_APPEND_DOUBLE(&p, "NCOLS", subXSize);
        BSON_APPEND_DOUBLE(&p, "NROWS", subYSize);
        BSON_APPEND_DOUBLE(&p, "XLLCENTER", rs->getXllCenter() + subbasin.xMin * cellSize);
        BSON_APPEND_DOUBLE(&p, "YLLCENTER", rs->getYllCenter() + (rs->getRows() - subbasin.yMax - 1) * cellSize);
        BSON_APPEND_DOUBLE(&p, "LAYERS", rs->getLayers());
        BSON_APPEND_DOUBLE(&p, "CELLSNUM", subbasin.cellCount);
        BSON_APPEND_UTF8(&p, "SRS", srs);

        char *databuf = (char *) subData;
        size_t datalength = sizeof(float) * subXSize * subYSize;
        MongoGridFS().writeStreamData(remoteFilename.str(), databuf, datalength, &p, gfs);
        if (NULL == MongoGridFS().getFile(remoteFilename.str(), gfs)) {
            // import failed! Terminate the subbasins loop and return false.
            flag = false;
            break;
        }
        bson_destroy(&p);

        databuf = NULL;
        Release1DArray(subData);
    }
    delete rs;
    return flag;
}

/*!
 * \brief Decomposite multi-layers raster data as 1D array, and import to MongoDB as GridFS
 * \param[in] bboxMap Map of subbasin extent box, key is subbasin id, value is \sa SubBasin object
 * \param[in] rsSubbasin Subbasin raster object, \sa clsRasterData
 * \param[in] coreName Core name of raster
 * \param[in] dstFiles Vector of raster full file paths
 * \param[in] conn MongoDB client object
 * \param[in] gfs MongoDB GridFS object
 * \return True if import successfully, otherwise return false.
 */
bool Decomposite2DRasterToMongoDB(map<int, SubBasin> &bboxMap, clsRasterData<int> *rsSubbasin, string coreName,
                                 vector<string> dstFiles, mongoc_client_t *conn, mongoc_gridfs_t *gfs) {
    bool flag = true;
    int colNum = dstFiles.size();
    clsRasterData<float> *rss = clsRasterData<float>::Init(dstFiles, false); // Do not calculate positions data
    if (nullptr == rss) exit(-1);
    int nXSize = rss->getCols();
    // int nYSize = rss.getRows();
    float noDataValue = rss->getNoDataValue();
    const char *srs = rss->getSRS();
    ///cout << nXSize << "\t" << nYSize << endl;
    float **rssData = rss->get2DRasterDataPointer();
    int *subbasinData = rsSubbasin->getRasterDataPointer();
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
        float *sub2DData = nullptr;
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
        float cellSize = rss->getCellWidth();
        MongoGridFS().removeFile(remoteFilename.str(), gfs);

        bson_t p = BSON_INITIALIZER;
        BSON_APPEND_INT32(&p, "SUBBASIN", id);
        BSON_APPEND_UTF8(&p, "TYPE", coreName.c_str());
        BSON_APPEND_UTF8(&p, "ID", remoteFilename.str().c_str());
        BSON_APPEND_UTF8(&p, "DESCRIPTION", coreName.c_str());
        BSON_APPEND_DOUBLE(&p, "CELLSIZE", cellSize);
        BSON_APPEND_DOUBLE(&p, "NODATA_VALUE", rss->getNoDataValue());
        BSON_APPEND_DOUBLE(&p, "NCOLS", subXSize);
        BSON_APPEND_DOUBLE(&p, "NROWS", subYSize);
        BSON_APPEND_DOUBLE(&p, "XLLCENTER", rss->getXllCenter() + subbasin.xMin * cellSize);
        BSON_APPEND_DOUBLE(&p, "YLLCENTER", rss->getYllCenter() + (rss->getRows() - subbasin.yMax - 1) * cellSize);
        BSON_APPEND_DOUBLE(&p, "LAYERS", rss->getLayers());
        BSON_APPEND_DOUBLE(&p, "CELLSNUM", subbasin.cellCount);
        BSON_APPEND_UTF8(&p, "SRS", srs);

        char *databuf = (char *) sub2DData;
        size_t datalength = sizeof(float) * subCellNum * colNum;
        MongoGridFS().writeStreamData(remoteFilename.str(), databuf, datalength, &p, gfs);
        if (NULL == MongoGridFS().getFile(remoteFilename.str(), gfs)) {
            // import failed! Terminate the subbasins loop and return false.
            flag = false;
            break;
        }
        bson_destroy(&p);
        databuf = nullptr;
        Release1DArray(sub2DData);
    }
    srs = nullptr;
    rssData = nullptr;
    subbasinData = nullptr;
    delete rss;
    return flag;
}

/*!
 * \brief Decomposite raster to separate files named suffixed by subbasin ID.
 *        If not for MPI version SEIMS, the whole basin data will be generated named suffixed by 0.
 * \param[in] bboxMap Map of subbasin extent box, key is subbasin id, value is \sa SubBasin object
 * \param[in] rsSubbasin Subbasin raster object, \sa clsRasterData
 * \param[in] dstFile Input raster full file path
 * \param[in] tmpFolder Folder to store the separated raster data file
 */
int DecompositeRaster(map<int, SubBasin> &bboxMap, clsRasterData<int> *rsSubbasin, const char *dstFile,
                      const char *tmpFolder) {
    clsRasterData<float>* rs = clsRasterData<float>::Init(dstFile, false);
    if (nullptr == rs) exit(-1);

    int nXSize = rs->getCols();
    // int nYSize = rs->getRows();
    float noDataValue = rs->getNoDataValue();

    float *rsData = rs->getRasterDataPointer();
    int *subbasinData = rsSubbasin->getRasterDataPointer();

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
        GDALDataset *poDstDS = poDriver->Create(subbasinFile.c_str(), subXSize, subYSize,
                                                1, GDT_Float32, papszOptions);

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
        float cellSize = rs->getCellWidth();
        geoTrans[0] = rs->getXllCenter() + (subbasin.xMin - 0.5f) * cellSize;
        geoTrans[1] = cellSize;
        geoTrans[2] = 0;
        geoTrans[3] = rs->getYllCenter() + (rs->getRows() - subbasin.yMin - 0.5f) * cellSize;
        geoTrans[4] = 0;
        geoTrans[5] = -cellSize;
        poDstDS->SetGeoTransform(geoTrans);

        CPLFree(subData);
        GDALClose(poDstDS);
    }
    delete rs;
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

    const char *subbasinFile = argv[1];
    const char *folder = argv[2];
    const char *modelName = argv[3];
    const char *gridFSName = argv[4];
    const char *hostname = argv[5];
    int port = atoi(argv[6]);
    const char *outTifFolder = nullptr;

    if (argc >= 8) {
        outTifFolder = argv[7];
    }
    //cout<<argc<<endl;
    //for(int i = 0; i < argc; i++)
    //	cout<<argv[i]<<endl;
    vector<string> dstFiles;
    FindFiles(folder, "*.tif", dstFiles);
    cout << "File number:" << dstFiles.size() << endl;
    //for (vector<string>::iterator it = dstFiles.begin(); it != dstFiles.end(); it++)
    //{
    //	cout<<*it<<",";
    //}
    /// Identify Array1D and Array2D dstFiles, respectively
    vector<string> coreFileNames;
    vector<string> array1DFiles;
    map<string, vector<string> > array2DFiles;
    map<string, vector<string> >::iterator array2DIter;
    for (auto it = dstFiles.begin(); it != dstFiles.end(); it++) {
        string tmpCoreName = GetCoreFileName(*it);
        coreFileNames.push_back(tmpCoreName);
        vector<string> tokens = SplitString(tmpCoreName, '_');
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
                    vector<string> tmpFileName;
                    tmpFileName.push_back(*it);
                    array2DFiles.insert(make_pair(coreVarName, tmpFileName));
                }
            } else {
                array1DFiles.push_back(*it);
            }
        }
    }
    vector<string> delVarNames;
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
    clsRasterData<int>* rsSubbasin = clsRasterData<int>::Init(subbasinFile, false);
    if (nullptr == rsSubbasin) exit(-1);
    map<int, SubBasin> bboxMap;
    FindBoundingBox(rsSubbasin, bboxMap);

    //////////////////////////////////////////////////////////////////////////
    /// loop to process the destination files

    /// connect to MongoDB
    MongoClient* client = MongoClient::Init(hostname, port);
    if (nullptr == client) exit(-1);
    mongoc_client_t *conn = client->getConn();
    mongoc_gridfs_t *gfs = client->getGridFS(string(modelName), string(gridFSName));

    cout << "Importing spatial data to MongoDB...\n";
    for (array2DIter = array2DFiles.begin(); array2DIter != array2DFiles.end(); array2DIter++) {
        vector<string> tmpFileNames = array2DIter->second;
        for (vector<string>::iterator it = tmpFileNames.begin(); it != tmpFileNames.end(); it++) {
            cout << "\t" << *it << endl;
            if (nullptr != outTifFolder) {
                DecompositeRaster(bboxMap, rsSubbasin, it->c_str(), outTifFolder);
            }
        }
        int loop = 1;
        while (loop < 3) {
            if (!Decomposite2DRasterToMongoDB(bboxMap, rsSubbasin, array2DIter->first, 
                                              tmpFileNames, conn, gfs)) {
                cout << "Import " << array2DIter->first << " failed, try time: " << loop << endl;
                loop++;
            }
            else{
                break;
            }
        }
        if (loop == 3) {
            cout << "ERROR! Exceed the max. try times please check and rerun!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (size_t i = 0; i < array1DFiles.size(); ++i) {
        cout << "\t" << array1DFiles[i] << endl;
        if (outTifFolder != nullptr) {
            DecompositeRaster(bboxMap, rsSubbasin, array1DFiles[i].c_str(), outTifFolder);
        }
        int loop = 1;
        while (loop < 3) {
            if (!DecompositeRasterToMongoDB(bboxMap, rsSubbasin, array1DFiles[i].c_str(), conn, gfs)) {
                cout << "Import " << array1DFiles[i] << " failed, try time: " << loop << endl;
                loop++;
            }
            else {
                break;
            }
        }
        if (loop == 3) {
            cout << "ERROR! Exceed max. try times please check and rerun!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    /// release
    mongoc_gridfs_destroy(gfs);
    delete client;
    delete rsSubbasin;
}
