/*!
 * \brief Utility functions of mongoDB
 * \author Junzhi Liu, LiangJun Zhu
 * \date May 2016
 *
 * 
 */
#pragma once

#include <mongoc.h>
#include <vector>
#include <set>
#include "util.h"
#include "ModelException.h"

using namespace std;

template<typename T> class clsRasterData;

/*!
 * \brief Get Integer value from \a bson_iter_t 
 *
 * The \a bson_type_t can be BSON_TYPE_INT32 or BSON_TYPE_INT64 or BSON_TYPE_UTF8
 *
 * \param[in] iter \a bson_iter_t
 * \return Integer value if success, or -1 if failed.
 */
int GetIntFromBSONITER(bson_iter_t *iter);

/*!
 * \brief Get Float value from \a bson_iter_t 
 *
 * The \a bson_type_t can be BSON_TYPE_INT32 or BSON_TYPE_INT64 or BSON_TYPE_DOUBLE or BSON_TYPE_UTF8
 *
 * \param[in] iter \a bson_iter_t
 * \return Float value if success, or -1.0 if failed.
 */
float GetFloatFromBSONITER(bson_iter_t *iter);
/*!
 * \brief Get Double value from \a bson_iter_t 
 *
 * The \a bson_type_t can be BSON_TYPE_INT32 or BSON_TYPE_INT64 or BSON_TYPE_DOUBLE or BSON_TYPE_UTF8
 *
 * \param[in] iter \a bson_iter_t
 * \return Double value if success, or -1.0 if failed.
 */
double GetDoubleFromBSONITER(bson_iter_t *iter);
/*!
 * \brief Get boolean value from \a bson_iter_t 
 *
 * The \a bson_type_t can be BSON_TYPE_INT32 or BSON_TYPE_INT64 or BSON_TYPE_DOUBLE or BSON_TYPE_UTF8
 *
 * \param[in] iter \a bson_iter_t
 * \return true if the value is 1, or greater than 0, or string matched to 'TURE', 'True', 'true' etc., or false.
 */
bool GetBoolFromBSONITER(bson_iter_t *iter);
/*!
 * \brief Get Date time from \a bson_iter_t 
 *
 * The \a bson_type_t can be BSON_TYPE_DATE_TIME
 *
 * \param[in] iter \a bson_iter_t 
 * \return Float value if success, or -1 if failed.
 */
time_t GetDateTimeFromBSONITER(bson_iter_t *iter);

/*!
 * \brief Get String from \a bson_iter_t 
 * The \a bson_type_t can be BSON_TYPE_UTF8
 *
 * \param[in] iter \a bson_iter_t 
 * \return Float value if success, or "" if failed.
 */
string GetStringFromBSONITER(bson_iter_t *iter);

/*!
 * \brief Get collection names in MongoDB database
 *
 * \param[in] conn \a mongoc_client_t
 * \param[in] dbName \string database name
 * \param[out] tableNameList \vector<string> collection names
 */
int GetCollectionNames(mongoc_client_t *conn, string &dbName, vector<string> &tableNameList);

/*!
 * \brief Get GridFs file names in MongoDB database
 *
 * \param[in] gfs \mongoc_gridfs_t GridFS
 * \return filenames vector<string>
 */
vector<string> GetGridFsFileNames(mongoc_gridfs_t *gfs);

/*!
 * \brief Read 1D array data from MongoDB database
 *
 * \param[in] spatialData \a mongoc_gridfs_t
 * \param[in] remoteFilename \string data file name
 * \param[out] num \int&, data length
 * \param[out] data \float*&, returned data
 */
extern void Read1DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &num, float *&data);
///*!
// * \brief Read 1D raster data from MongoDB database
// *
// * \param[in] spatialData \a mongoc_gridfs_t
// * \param[in] remoteFilename \string data file name
// * \param[in] templateRaster \clsRasterData*
// * \param[out] num \int&, data length
// * \param[out] data \float*&, returned data
// * \sa Read1DArrayFromMongoDB()
// */
//extern void Read1DRasterFromMongoDB(mongoc_gridfs_t* spatialData, string& remoteFilename, clsRasterData* templateRaster, int& num, float*& data);

/*!
 * \brief Read 2D array data from MongoDB database
 * The matrix format is as follows:
 *                                     5                 (Row number)
 *          RowIdx\ColIdx	0	1 2	3	4
					0					1	9.
					1					2	8.	1.
					2					3	5.	2.
					3					1	2.
					4					4	2.	5.	1.	8.
	i.e., the first element in each row is the valid number of the current row.
            
 * \param[in] spatialData \a mongoc_gridfs_t
 * \param[in] remoteFilename \string data file name
 * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
 * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
 * \param[out] data \float**&, returned data
 */
extern void Read2DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &rows, int& cols, float **&data);
///*!
// * \brief Read 2D raster data from MongoDB database
// *
// * \param[in] spatialData \a mongoc_gridfs_t
// * \param[in] remoteFilename \string data file name
// * \param[in] templateRaster \clsRasterData*
// * \param[out] num \int&,  valid cell numbers of single layer raster
// * \param[out] lyrs \int&,  valid cell numbers of single layer raster
// * \param[out] data \float**&, returned data
// * \sa Read2DArrayFromMongoDB()
// */
//extern void Read2DRasterFromMongoDB(mongoc_gridfs_t* spatialData, string& remoteFilename, clsRasterData* templateRaster, int& num, int& lyrs, float*& data);

///*!
// * \brief Read 2D soil attribute data from MongoDB database
// *
// * \param[in] spatialData \a mongoc_gridfs_t
// * \param[in] remoteFilename \string data file name
// * \param[in] templateRaster \clsRasterData*
// * \param[out] n \int&, valid cell number 
// * \param[out] data \float*&, returned data
// * \deprecated Replaced by \sa Read2DArrayFromMongoDB()
// */
//extern void Read2DSoilAttr(mongoc_gridfs_t* spatialData, string remoteFilename, clsRasterData* templateRaster, int& n, float**& data);
/*!
 * \brief Read IUH data from MongoDB database
 * Not sure the different with \sa Read2DArrayFromMongoDB
 * \param[in] spatialData \a mongoc_gridfs_t
 * \param[in] remoteFilename \string data file name
 * \param[out] n \int&, valid cell number
 * \param[out] data \float*&, returned data
 */
extern void ReadIUHFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &n, float **&data);

/*!
 * \brief Read Longterm multi reach information from MongoDB database
 * Assume the reaches table contains all the reaches information
 * \param[in] conn \a mongoc_client_t
 * \param[in] dbName model name, which contains parameters and spatial database
 * \param[out] nr Field number in REACH table
 * \param[out] nc Number of reaches
 * \param[out] data \float*&, returned data
 */
extern void ReadLongTermMutltiReachInfo(mongoc_client_t *conn, string &dbName, int &nr, int &nc, float **&data);

/*!
 * \brief Read Longterm reach information from MongoDB database
 * \param[in] conn \a mongoc_client_t
 * \param[in] dbName model name, which contains parameters and spatial database
 * \param[in] subbasinID subbasin ID
 * \param[out] nr Field number in REACH table
 * \param[out] nc Number of reaches
 * \param[out] data \float*&, returned data
 */
extern void ReadLongTermReachInfo(mongoc_client_t *conn, string &dbName, int subbasinID, int &nr, int &nc,
                                  float **&data);

/*!
 * \brief Read multi reach information from MongoDB database
 * Assume the reaches table contains all the reaches information
 * \param[in] layeringMethod \sa LayeringMethod
 * \param[in] conn \a mongoc_client_t
 * \param[in] dbName model name, which contains parameters and spatial database
 * \param[out] nr Field number in REACH table
 * \param[out] nc Number of reaches
 * \param[out] data \float*&, returned data
 */
extern void ReadMutltiReachInfoFromMongoDB(LayeringMethod layeringMethod, mongoc_client_t *conn, string &dbName,
                                           int &nr, int &nc, float **&data);

/*!
 * \brief Read single reach information from MongoDB database
 * Assume the reaches table contains all the reaches information
 * \param[in] layeringMethod \sa LayeringMethod
 * \param[in] conn \a mongoc_client_t
 * \param[in] dbName model name, which contains parameters and spatial database
 * \param[in] subbasinID subbasin ID
 * \param[out] nr Field number in REACH table
 * \param[out] nc Number of reaches
 * \param[out] data \float*&, returned data
 */
extern void ReadReachInfoFromMongoDB(LayeringMethod layeringMethod, mongoc_client_t *conn, string &dbName,
                                     int nSubbasin, int &nr, int &nc, float **&data);

