/*----------------------------------------------------------------------
*	Purpose:  Raster Data
*
*	Created:	Junzhi Liu
*	Date:		29-July-2012
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/


#include <fstream>
#include <string>
#include "GridLayering.h"
#include <iostream>

#include "mongo.h"
#include "bson.h"

using namespace std;


void ReadArcAscii(const char *filename, RasterHeader &rs, int *&data)
{
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
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        rasterFile >> data[i * nCols + j];
                }
        }

        rasterFile.close();

}


void OutputArcAscii(const char *filename, RasterHeader &rs, int *data, int noDataValue)
{
        ofstream rasterFile(filename);
        //write header
        rasterFile << "NCOLS " << rs.nCols << "\n";
        rasterFile << "NROWS " << rs.nRows << "\n";
        rasterFile << "XLLCENTER " << rs.xll << "\n";
        rasterFile << "YLLCENTER " << rs.yll << "\n";
        rasterFile << "CELLSIZE " << rs.dx << "\n";
        rasterFile << "NODATA_VALUE " << noDataValue << "\n";

        //write file
        for (int i = 0; i < rs.nRows; ++i)
        {
                for (int j = 0; j < rs.nCols; ++j)
                {
                        rasterFile << data[i * rs.nCols + j] << "\t";
                }
                rasterFile << "\n";
        }

        rasterFile.close();
}


void ReadFromMongo(gridfs *gfs, const char *remoteFilename, RasterHeader &rs, int *&data)
{
        gridfile gfile[1];
        bson b[1];
        bson_init(b);
        bson_append_string(b, "filename", remoteFilename);
        bson_finish(b);
        int flag = gridfs_find_query(gfs, b, gfile);
        if (0 != flag)
        {
                cout << "Failed in ReadFromMongoDB, Remote file: " << remoteFilename << endl;
        }

        size_t length = (size_t) gridfile_get_contentlength(gfile);
        char *buf = (char *) malloc(length);
        gridfile_read(gfile, length, buf);
        float *dataMongo = (float *) buf;

        bson bmeta[1];
        gridfile_get_metadata(gfile, bmeta);
        bson_iterator iterator[1];
        if (bson_find(iterator, bmeta, "NCOLS"))
                rs.nCols = bson_iterator_int(iterator);
        if (bson_find(iterator, bmeta, "NROWS"))
                rs.nRows = bson_iterator_int(iterator);
        if (bson_find(iterator, bmeta, "NODATA_VALUE"))
                rs.noDataValue = int(bson_iterator_double(iterator));
        if (bson_find(iterator, bmeta, "XLLCENTER"))
                rs.xll = (float) bson_iterator_double(iterator);
        if (bson_find(iterator, bmeta, "YLLCENTER"))
                rs.yll = (float) bson_iterator_double(iterator);
        if (bson_find(iterator, bmeta, "CELLSIZE"))
                rs.dx = (float) bson_iterator_double(iterator);

        //bson_destroy(bmeta);

        int n = rs.nRows * rs.nCols;
        data = new int[n];

        //read file
        for (int i = 0; i < n; i++)
                data[i] = dataMongo[i];

        bson_destroy(b);
        gridfile_destroy(gfile);
        //gridfs_destroy(gfs);
        //mongo_destroy(conn);
        free(buf);
}

void ReadFromMongoFloat(gridfs *gfs, const char *remoteFilename, RasterHeader &rs, float *&data)
{
        gridfile gfile[1];
        bson b[1];
        bson_init(b);
        bson_append_string(b, "filename", remoteFilename);
        bson_finish(b);
        int flag = gridfs_find_query(gfs, b, gfile);
        if (0 != flag)
        {
                cout << "Failed in ReadFromMongoDB, Remote file: " << remoteFilename << endl;
        }

        bson bmeta[1];
        gridfile_get_metadata(gfile, bmeta);
        bson_iterator iterator[1];
        if (bson_find(iterator, bmeta, "NCOLS"))
                rs.nCols = bson_iterator_int(iterator);
        if (bson_find(iterator, bmeta, "NROWS"))
                rs.nRows = bson_iterator_int(iterator);
        if (bson_find(iterator, bmeta, "NODATA_VALUE"))
                rs.noDataValue = int(bson_iterator_double(iterator));
        if (bson_find(iterator, bmeta, "XLLCENTER"))
                rs.xll = (float) bson_iterator_double(iterator);
        if (bson_find(iterator, bmeta, "YLLCENTER"))
                rs.yll = (float) bson_iterator_double(iterator);
        if (bson_find(iterator, bmeta, "CELLSIZE"))
                rs.dx = (float) bson_iterator_double(iterator);

        size_t n = rs.nRows * rs.nCols;
        size_t length = (size_t) gridfile_get_contentlength(gfile);
        if (4 * n != length)
        {
                cout << "The data length of " << remoteFilename << " is not consistant with metadata.\n";
                bson_destroy(b);
                gridfile_destroy(gfile);
                return;
        }

        data = new float[n];
        char *buf = (char *) data;
        gridfile_read(gfile, length, buf);

        bson_destroy(b);
        gridfile_destroy(gfile);
}

int WriteStringToMongoDB(gridfs *gfs, int id, const char *type, int number, const char *s)
{
        bson *p = (bson *) malloc(sizeof(bson));
        bson_init(p);
        bson_append_int(p, "SUBBASIN", id);
        bson_append_string(p, "TYPE", type);

        char remoteFilename[100];
        sprintf(remoteFilename, "%d_%s", id, type);

        bson_append_string(p, "ID", remoteFilename);
        bson_append_string(p, "DESCRIPTION", type);
        bson_append_double(p, "NUMBER", number);
        bson_finish(p);

        gridfile gfile[1];
        /// If the file is already existed in MongoDB, if existed, then delete it!
        gridfs_remove_filename(gfs, remoteFilename);
        /// create a new one
        int n = number * 4;
        int index = 0;
        gridfile_writer_init(gfile, gfs, remoteFilename, type);

        while (index < n)
        {
                int dataLen = 1024;
                if (n - index < dataLen)
                        dataLen = n - index;
                gridfile_write_buffer(gfile, s + index, dataLen);
                index += dataLen;
        }

        gridfile_set_metadata(gfile, p);
        int flag = gridfile_writer_done(gfile);
        if (flag == MONGO_ERROR) /// if write to MongoDB failed.
        {
                cout<<"\t"<<string(remoteFilename)<<" output failed, please retry!"<<endl;
        }
        gridfile_destroy(gfile);

        bson_destroy(p);
        free(p);

        return flag;
}

void OutputLayersToMongoDB(const char *layeringTxtFile, const char *dataType, int id, gridfs *gfs)
{
        ifstream ifs(layeringTxtFile);
        int nValidGrids, nLayers;
        ifs >> nValidGrids >> nLayers;

        int n = nValidGrids + nLayers + 1;
        float *pLayers = new float[n];
        pLayers[0] = nLayers;
        int index = 1;

        for (int i = 1; i < n; ++i)
                ifs >> pLayers[i];
        ifs.close();

        WriteStringToMongoDB(gfs, id, dataType, n, (const char *) pLayers);
        delete[] pLayers;
        pLayers = NULL;
}
