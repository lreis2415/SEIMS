#include <vector>
#include <map>
#include <string.h>
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

//gdal
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"

//mongodb
#include "mongo.h"
#include "bson.h"
#include "gridfs.h"

//this project
#include "util.h"
#include "SubBasin.h"
#include "Raster.cpp"

using namespace std;

int FindBoundingBox(Raster<int> &rsSubbasin, map<int, SubBasin> &bboxMap)
{
    int nXSize = rsSubbasin.GetNumberofColumns();
    int nYSize = rsSubbasin.GetNumberOfRows();

    int **pData = rsSubbasin.GetData();
    int noDataValue = rsSubbasin.GetNoDataValue();
    // find bounding box for each subbasin
    for (int i = 0; i < nYSize; i++)
    {
        for (int j = 0; j < nXSize; j++)
        {
            int id = pData[i][j];
            if (noDataValue != id)
            {
                if (bboxMap.find(id) == bboxMap.end())
                {
                    bboxMap[id] = SubBasin(j, i, j, i);
                }
                else
                {
                    if (j < bboxMap[id].xMin)
                        bboxMap[id].xMin = j;
                    else if (j > bboxMap[id].xMax)
                        bboxMap[id].xMax = j;

                    if (i > bboxMap[id].yMax)
                        bboxMap[id].yMax = i;
                }
                bboxMap[id].cellCount += 1;
            }
        }
    }
    return 0;
}


int DecompositeRasterToMongoDB(map<int, SubBasin> &bboxMap, Raster<int> &rsSubbasin, const char *dstFile, mongo *conn,
                               gridfs *gfs,
                               vector<string> fileExisted)
{
    Raster<float> rs;
    rs.ReadFromGDAL(dstFile);

    int nXSize = rs.GetNumberofColumns();
    int nYSize = rs.GetNumberOfRows();
    float noDataValue = rs.GetNoDataValue();
    const char *srs = rs.GetSRS();
    //cout << nXSize << "\t" << nYSize << endl;
    float **rsData = rs.GetData();
    int **subbasinData = rsSubbasin.GetData();

    map<int, SubBasin>::iterator it;
    string coreName = GetCoreFileName(dstFile);

    for (it = bboxMap.begin(); it != bboxMap.end(); it++)
    {
        int id = it->first;
		int subbasinID = id;
		if (bboxMap.size() == 1)
			id = 0;
        SubBasin &subbasin = it->second;
        int subXSize = subbasin.xMax - subbasin.xMin + 1;
        int subYSize = subbasin.yMax - subbasin.yMin + 1;

        float *subData = new float[subXSize * subYSize];

        for (int i = subbasin.yMin; i <= subbasin.yMax; i++)
        {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++)
            {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[i][j] == subbasinID)
                    subData[subIndex] = rsData[i][j];
                else
                    subData[subIndex] = noDataValue;
            }
        }

        ostringstream remoteFilename;
        remoteFilename << id << "_" << GetUpper(coreName);
        float cellSize = rs.GetXCellSize();

        if (find(fileExisted.begin(), fileExisted.end(), remoteFilename.str()) != fileExisted.end())
            gridfs_remove_filename(gfs, remoteFilename.str().c_str());

        bson *p = (bson *) malloc(sizeof(bson));
        bson_init(p);
        bson_append_int(p, "SUBBASIN", id);
        bson_append_string(p, "TYPE", coreName.c_str());
        bson_append_string(p, "ID", remoteFilename.str().c_str());
        bson_append_string(p, "DESCRIPTION", coreName.c_str());
        bson_append_double(p, "CELLSIZE", cellSize);
        bson_append_double(p, "NODATA_VALUE", rs.GetNoDataValue());
        bson_append_double(p, "NCOLS", subXSize);
        bson_append_double(p, "NROWS", subYSize);
        bson_append_double(p, "XLLCENTER", rs.GetXllCenter() + subbasin.xMin * cellSize);
        bson_append_double(p, "YLLCENTER", rs.GetYllCenter() + (rs.GetNumberOfRows() - subbasin.yMax - 1) * cellSize);
        bson_append_double(p, "LAYERS", 1.0);
        bson_append_string(p, "SRS", srs);
        bson_finish(p);

        gridfile gfile[1];
        gridfile_writer_init(gfile, gfs, remoteFilename.str().c_str(), "float");
        for (int k = 0; k < subYSize; k++)
        {
            gridfile_write_buffer(gfile, (const char *) (subData + subXSize * k), sizeof(float) * subXSize);
        }
        gridfile_set_metadata(gfile, p);
        gridfile_writer_done(gfile);
        gridfile_destroy(gfile);

        bson_destroy(p);
        free(p);
        delete[] subData;
        subData = NULL;
    }
    return 0;
}

int Decomposite2DRasterToMongoDB(map<int, SubBasin> &bboxMap, Raster<int> &rsSubbasin, string coreName,
                                 vector<string> dstFiles,
                                 mongo *conn, gridfs *gfs, vector<string> fileExisted)
{
    int colNum = dstFiles.size();
    vector<Raster<float> > rss(colNum);
    for (int i = 0; i < colNum; i++)
    {
        Raster<float> rs;
        rs.ReadFromGDAL(dstFiles[i].c_str());
        rss[i].Copy(rs);
    }

    int nXSize = rss[0].GetNumberofColumns();
    int nYSize = rss[0].GetNumberOfRows();
    float noDataValue = rss[0].GetNoDataValue();
    const char *srs = rss[0].GetSRS();
    ///cout << nXSize << "\t" << nYSize << endl;
    vector<float **> rssData(colNum);
    for (int i = 0; i < colNum; i++)
        rssData[i] = rss[i].GetData();
    int **subbasinData = rsSubbasin.GetData();
    map<int, SubBasin>::iterator it;
    for (it = bboxMap.begin(); it != bboxMap.end(); it++)
    {
        int id = it->first;
		int subbasinID = id;
		if (bboxMap.size() == 1)
			id = 0;
        SubBasin &subbasin = it->second;
        int subXSize = subbasin.xMax - subbasin.xMin + 1;
        int subYSize = subbasin.yMax - subbasin.yMin + 1;

        float **sub2DData = new float *[subXSize * subYSize];
        for (int i = 0; i < subXSize * subYSize; i++)
            sub2DData[i] = new float[colNum];

        for (int i = subbasin.yMin; i <= subbasin.yMax; i++)
        {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++)
            {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[i][j] == subbasinID)
                {
                    for (int k = 0; k < colNum; k++)
                        sub2DData[subIndex][k] = rssData[k][i][j];
                }
                else
                    for (int k = 0; k < colNum; k++)
                        sub2DData[subIndex][k] = noDataValue;
            }
        }

        ostringstream remoteFilename;
        remoteFilename << id << "_" << GetUpper(coreName);
        float cellSize = rss[0].GetXCellSize();

        if (find(fileExisted.begin(), fileExisted.end(), remoteFilename.str()) != fileExisted.end())
            gridfs_remove_filename(gfs, remoteFilename.str().c_str());

        bson *p = (bson *) malloc(sizeof(bson));
        bson_init(p);
        bson_append_int(p, "SUBBASIN", id);
        bson_append_string(p, "TYPE", coreName.c_str());
        bson_append_string(p, "ID", remoteFilename.str().c_str());
        bson_append_string(p, "DESCRIPTION", coreName.c_str());
        bson_append_double(p, "CELLSIZE", cellSize);
        bson_append_double(p, "NODATA_VALUE", noDataValue);
        bson_append_double(p, "NCOLS", subXSize);
        bson_append_double(p, "NROWS", subYSize);
        bson_append_double(p, "XLLCENTER", rss[0].GetXllCenter() + subbasin.xMin * cellSize);
        bson_append_double(p, "YLLCENTER",
                           rss[0].GetYllCenter() + (rss[0].GetNumberOfRows() - subbasin.yMax - 1) * cellSize);
        bson_append_double(p, "LAYERS", colNum);
        bson_append_string(p, "SRS", srs);
        bson_finish(p);

        gridfile gfile[1];
        gridfile_writer_init(gfile, gfs, remoteFilename.str().c_str(), "float");
        for (int k = 0; k < subYSize * subXSize; k++)
        {
            gridfile_write_buffer(gfile, (const char *) (sub2DData[k]), sizeof(float) * colNum);
        }
        gridfile_set_metadata(gfile, p);
        gridfile_writer_done(gfile);
        gridfile_destroy(gfile);

        bson_destroy(p);
        free(p);
        for (int i = 0; i < subXSize * subYSize; i++)
            delete[] sub2DData[i];
        delete[] sub2DData;
        sub2DData = NULL;
    }
    return 0;
}

int DecompositeRaster(map<int, SubBasin> &bboxMap, Raster<int> &rsSubbasin, const char *dstFile, const char *tmpFolder)
{
    Raster<float> rs;
    rs.ReadFromGDAL(dstFile);

    int nXSize = rs.GetNumberofColumns();
    int nYSize = rs.GetNumberOfRows();
    float noDataValue = rs.GetNoDataValue();

    float **rsData = rs.GetData();
    int **subbasinData = rsSubbasin.GetData();

    const char *pszFormat = "GTiff";
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    map<int, SubBasin>::iterator it;
    string coreName = GetCoreFileName(dstFile);
    for (it = bboxMap.begin(); it != bboxMap.end(); it++)
    {
        int id = it->first;
		int subbasinID = id;
		if (bboxMap.size() == 1) // means not for Cluster
			id = 0;
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
        for (int i = subbasin.yMin; i <= subbasin.yMax; i++)
        {
            for (int j = subbasin.xMin; j <= subbasin.xMax; j++)
            {
                int index = i * nXSize + j;
                int subIndex = (i - subbasin.yMin) * subXSize + (j - subbasin.xMin);
                if (subbasinData[i][j] == subbasinID)
                    subData[subIndex] = rsData[i][j];
                else
                    subData[subIndex] = noDataValue;
            }
        }

        //write the data to new file
        GDALRasterBand *poDstBand = poDstDS->GetRasterBand(1);
        poDstBand->RasterIO(GF_Write, 0, 0, subXSize, subYSize,
                            subData, subXSize, subYSize, GDT_Float32, 0, 0);
        poDstBand->SetNoDataValue(noDataValue);

        double geoTrans[6];
        float cellSize = rs.GetXCellSize();
        geoTrans[0] = rs.GetXllCenter() + (subbasin.xMin - 0.5f) * cellSize;
        geoTrans[1] = cellSize;
        geoTrans[2] = 0;
        geoTrans[3] = rs.GetYllCenter() + (rs.GetNumberOfRows() - subbasin.yMin - 0.5f) * cellSize;
        geoTrans[4] = 0;
        geoTrans[5] = -cellSize;
        poDstDS->SetGeoTransform(geoTrans);

        CPLFree(subData);
        GDALClose(poDstDS);

        // import the subbasin file to BeyondDB
        string desc = "";
        //ImportToBeyondDB(subbasinFile, id, coreName, subbasin.cellCount, subXSize, subYSize);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 7)
    {
        cout << "Usage: " <<
        "ImportRaster <MaskFile> <DataFolder> <modelName> <GridFSName> <hostIP> <port> [outputFolder]\n";
        exit(-1);
    }

    GDALAllRegister();

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
	
    if (argc >= 8)
        outTifFolder = argv[7];
	//cout<<argc<<endl;
	//for(int i = 0; i < argc; i++)
	//	cout<<argv[i]<<endl;
    vector<string> dstFiles;
#ifndef linux
    FindFiles(folder, "*.tif", dstFiles);
#else
	FindFiles(folder, ".tif", dstFiles);
#endif
	cout << "File number:"<<dstFiles.size()<<endl;
	//for (vector<string>::iterator it = dstFiles.begin(); it != dstFiles.end(); it++)
	//{
	//	cout<<*it<<",";
	//}
    /// Identify Array1D and Array2D dstFiles, respectively
    vector<string> coreFileNames;
    vector<string> array1DFiles;
    map<string, vector<string> > array2DFiles;
    map<string, vector<string> >::iterator array2DIter;
    for (vector<string>::iterator it = dstFiles.begin(); it != dstFiles.end(); it++)
    {
        string tmpCoreName = GetCoreFileName(*it);
        coreFileNames.push_back(tmpCoreName);
        vector<string> tokens = SplitString(tmpCoreName, '_');
        int length = tokens.size();

        if (length <= 1)
            array1DFiles.push_back(*it);
        else if (length >= 2) /// there are more than one underscore exist
        {
            string::size_type end = tmpCoreName.find_last_of("_");
            string coreVarName = tmpCoreName.substr(0, end);

            if (atoi(tokens[length - 1].c_str()))
            {
                array2DIter = array2DFiles.find(coreVarName);
                if (array2DIter != array2DFiles.end())
                {
                    array2DFiles[coreVarName].push_back(*it);
                }
                else
                {
                    vector<string> tmpFileName;
                    tmpFileName.push_back(*it);
                    array2DFiles.insert(make_pair(coreVarName, tmpFileName));
                }
            }
            else
                array1DFiles.push_back(*it);
        }
    }
    vector<string> delVarNames;
    for (array2DIter = array2DFiles.begin(); array2DIter != array2DFiles.end(); array2DIter++)
    {
        if (array2DIter->second.size() == 1)
        {
            array1DFiles.push_back(array2DIter->second.at(0));
            delVarNames.push_back(array2DIter->first);
        }
        else
            sort(array2DIter->second.begin(), array2DIter->second.end());
    }
    for (vector<string>::iterator it = delVarNames.begin(); it != delVarNames.end(); it++)
        array2DFiles.erase(*it);
    vector<string>(array1DFiles).swap(array1DFiles);

    //////////////////////////////////////////////////////////////////////////
    // read the subbasin file, and find the bounding box of each subbasin
    Raster<int> rsSubbasin;
    rsSubbasin.ReadFromGDAL(subbasinFile);
    map<int, SubBasin> bboxMap;
    FindBoundingBox(rsSubbasin, bboxMap);

    //////////////////////////////////////////////////////////////////////////
    /// loop to process the destination files

    /// connect to MongoDB
    mongo conn[1];
    gridfs gfs[1];
    int status = mongo_connect(conn, hostname, port);

    if (MONGO_OK != status)
    {
        cout << "can not connect to MongoDB.\n";
        exit(-1);
    }
    gridfs_init(conn, modelName, gridFSName, gfs);

    /// read document already existed in "spatial" collection
    mongo_cursor cursor[1];
    bson query[1];
    bson_init(query);
    bson_finish(query);
    char spatialCollection[255];
#ifndef linux
    strcpy_s(spatialCollection, modelName);
    strcat_s(spatialCollection, ".");
    strcat_s(spatialCollection, gridFSName);
    strcat_s(spatialCollection, ".files");
#else
	strcpy(spatialCollection, modelName);
	strcat(spatialCollection, ".");
	strcat(spatialCollection, gridFSName);
	strcat(spatialCollection, ".files");
#endif
    mongo_cursor_init(cursor, conn, spatialCollection);
    mongo_cursor_set_query(cursor, query);
    vector<string> fileNamesExisted;
    while (mongo_cursor_next(cursor) == MONGO_OK)
    {
        bson_iterator iter[1];
        //bson_print(&cursor->current);
        if (bson_find(iter, &cursor->current, "filename"))
            fileNamesExisted.push_back(string(bson_iterator_string(iter)));
    }

    cout << "Importing spatial data to MongoDB...\n";
    for (array2DIter = array2DFiles.begin(); array2DIter != array2DFiles.end(); array2DIter++)
    {
        vector<string> tmpFileNames = array2DIter->second;
        for (vector<string>::iterator it = tmpFileNames.begin(); it != tmpFileNames.end(); it++)
        {
            cout << "\t" << *it << endl;
            if (outTifFolder != NULL)
                DecompositeRaster(bboxMap, rsSubbasin, it->c_str(), outTifFolder);
        }
        Decomposite2DRasterToMongoDB(bboxMap, rsSubbasin, array2DIter->first, tmpFileNames, conn, gfs,
                                     fileNamesExisted);
    }
    for (size_t i = 0; i < array1DFiles.size(); ++i)
    {
        cout << "\t" << array1DFiles[i] << endl;
        if (outTifFolder != NULL)
            DecompositeRaster(bboxMap, rsSubbasin, array1DFiles[i].c_str(), outTifFolder);
        DecompositeRasterToMongoDB(bboxMap, rsSubbasin, array1DFiles[i].c_str(), conn, gfs, fileNamesExisted);
    }

    gridfs_destroy(gfs);
    mongo_destroy(conn);
}

