#include "utilities.h"
#include "MongoUtil.h"
#include "seims.h"
#include "text.h"

/*!
 * \brief Read 1D array data from MongoDB database
 *
 * \param[in] spatialData \a mongoc_gridfs_t
 * \param[in] remoteFilename \string data file name
 * \param[out] num \int&, data length
 * \param[out] data \float*&, returned data
 */
extern void Read1DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &num, float *&data);

/*!
 * \brief Read 2D array data from MongoDB database
 * The matrix format is as follows:
 *                                      5                 (Row number)
 *          RowIdx\ColIdx	0	1 2	3	4
					0					1	9.
					1					2	8.	1.
					2					2	5.	2.
					3					1	2.
					4					4	2.	5.	1.	8.
	i.e., the first element in each row is the valid number of the current row.
            
 * \param[in] spatialData \a mongoc_gridfs_t
 * \param[in] remoteFilename \string data file name
 * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
 * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
 * \param[out] data \float**&, returned data
 */
extern void
Read2DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &rows, int &cols, float **&data);

/*!
 * \brief Read IUH data from MongoDB database
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
extern void ReadLongTermMultiReachInfo(mongoc_client_t *conn, string &dbName, int &nr, int &nc, float **&data);

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
