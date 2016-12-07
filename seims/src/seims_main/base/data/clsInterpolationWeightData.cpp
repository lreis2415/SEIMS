/*!
 * \brief Methods for clsInterpolationWeightData class
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May. 2016
 *
 * 
 */
#include "clsInterpolationWeightData.h"
#include <fstream>
#include "utils.h"
#include "util.h"
#include "ModelException.h"
//#include <string>
//#include <iostream>
#include "MongoUtil.h"

using namespace std;

clsInterpolationWeightData::clsInterpolationWeightData(string weightFileName)
{
    m_fileName = weightFileName;
    m_weightData = NULL;
}

clsInterpolationWeightData::clsInterpolationWeightData(mongoc_gridfs_t *gfs, const char *remoteFilename)
{
    m_fileName = remoteFilename;
    m_weightData = NULL;
    ReadFromMongoDB(gfs, remoteFilename);
}

clsInterpolationWeightData::~clsInterpolationWeightData(void)
{
    if (m_weightData != NULL)
    {
        delete[] m_weightData;
    }
}

void clsInterpolationWeightData::getWeightData(int *n, float **data)
{
    *n = m_nRows;
    *data = m_weightData;
}

void clsInterpolationWeightData::dump(ostream *fs)
{
    if (fs == NULL) return;

    int index = 0;
    for (int i = 0; i < m_nRows; ++i)
    {
        //rasterFile >> tmp >> tmp;
        for (int j = 0; j < m_nCols; ++j)
        {
            index = i * m_nCols + j;
            *fs << m_weightData[index] << "\t";
        }
        *fs << endl;
    }
}

void clsInterpolationWeightData::dump(string fileName)
{
    ofstream fs;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open())
    {
        dump(&fs);
        fs.close();
    }
}

void clsInterpolationWeightData::ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename)
{
    //clock_t start = clock();
    mongoc_gridfs_file_t *gfile = NULL;
    bson_error_t *err = NULL;
    gfile = mongoc_gridfs_find_one_by_filename(gfs, remoteFilename, err);

    if (err != NULL || gfile == NULL)  /// no query result exists
    {
        string filename = remoteFilename;
        int index = filename.find_last_of('_');
        string type = filename.substr(index + 1);
        //bool flag = true;
        if (StringMatch(type, DataType_PotentialEvapotranspiration) || StringMatch(type, DataType_SolarRadiation)
            || StringMatch(type, DataType_RelativeAirMoisture) || StringMatch(type, DataType_MeanTemperature)
            || StringMatch(type, DataType_MaximumTemperature) || StringMatch(type, DataType_MinimumTemperature))
        {
            string meteoFileName = filename.substr(0, index + 1) + DataType_Meteorology;
            gfile = mongoc_gridfs_find_one_by_filename(gfs, meteoFileName.c_str(), err);
            if (err != NULL || gfile == NULL)
            {
                throw ModelException("clsInterpolationWeightData", "ReadFromMongoDB",
                                     "Failed in finding GridFS file: " + string(remoteFilename) + ".\n");
            }
        }
    }
	// cout << remoteFilename << endl;
    size_t length = (size_t) mongoc_gridfs_file_get_length(gfile);
    m_weightData = new float[length / 4];
    mongoc_iovec_t iov;
    iov.iov_base = (char *) m_weightData;
    iov.iov_len = length;
    mongoc_stream_t *stream;
    stream = mongoc_stream_gridfs_new(gfile);
	//ssize_t r = mongoc_stream_readv(stream, &iov, 1, -1, 0); /// m_weightData read completed!
	mongoc_stream_readv(stream, &iov, 1, -1, 0); /// m_weightData read completed!
    /// Get metadata
    const bson_t *md;
    md = mongoc_gridfs_file_get_metadata(gfile);
    /// Get value of given keys
    bson_iter_t iter;
    if (bson_iter_init(&iter, md) && bson_iter_find(&iter, MONG_GRIDFS_WEIGHT_CELLS))
    {
        m_nRows = GetIntFromBSONITER(&iter);
    }
    else
        throw ModelException("clsInterpolationWeightData", "ReadFromMongoDB",
                             "Failed in get INT value: " + string(MONG_GRIDFS_WEIGHT_CELLS) + "\n");
    if (bson_iter_init(&iter, md) && bson_iter_find(&iter, MONG_GRIDFS_WEIGHT_SITES))
    {
        m_nCols = GetIntFromBSONITER(&iter);
    }
    else
        throw ModelException("clsInterpolationWeightData", "ReadFromMongoDB",
                             "Failed in get INT value: " + string(MONG_GRIDFS_WEIGHT_SITES) + "\n");
    mongoc_gridfs_file_destroy(gfile);
}


/// old code using mongoDB 2.x and mongo-c-driver 0.6, by LJ, May 5, 2016
//gridfile gfile[1];
//bson b[1];
//bson_init(b);
//bson_append_string(b, MONG_GRIDFS_FN,  remoteFilename);
//bson_finish(b);
//int flag = gridfs_find_query(gfs, b, gfile);
//bson_destroy(b);

//if (flag != 0)
//{
//	string filename = remoteFilename;
//	int index = filename.find_last_of('_');
//	string type = filename.substr(index+1);
//	if (StringMatch(type, "PET") || StringMatch(type, "SR") || StringMatch(type, "SM") || StringMatch(type, "T")
//		|| StringMatch(type, "TMax") || StringMatch(type, "TMin"))
//	{
//		string meteoFileName = filename.substr(0, index+1) +  "M";
//		bson bm[1];
//		bson_init(bm);
//		bson_append_string(bm, "filename",  meteoFileName.c_str());
//		bson_finish(bm);
//		flag = gridfs_find_query(gfs, bm, gfile);
//	}
//}
//

//if(0 != flag)
//	throw ModelException("clsInterpolationWeightData", "ReadFromMongoDB", "Failed in gridfs_find_query file: " + string(remoteFilename) + ".\n");

//size_t length = (size_t)gridfile_get_contentlength(gfile);
////char* buf = (char*)malloc(length);
//m_weightData = new float[length/4];
//char* buf = (char*)m_weightData;
//gridfile_read (gfile, length, buf);
//
//bson bmeta[1];
//gridfile_get_metadata(gfile, bmeta);
//bson_iterator iterator[1];
//if ( bson_find( iterator, bmeta, "NUM_CELLS" ))
//	m_nRows = bson_iterator_int(iterator);
//if ( bson_find( iterator, bmeta, "NUM_SITES" ))
//	m_nCols = bson_iterator_int(iterator);

//gridfile_destroy(gfile);
