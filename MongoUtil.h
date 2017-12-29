/*!
 * \brief Utility functions of MongoDB
 *        Currently, mongo-c-driver 1.3.5 or later is supported.
 * \author Junzhi Liu, LiangJun Zhu
 * \date May 2016
 * \revised Feb 2017
 *          Nov 2017 lj Add unittest based on gtest/gmock.
 * \note No exceptions will be thrown.
 */
#ifndef MONGO_UTILS
#define MONGO_UTILS

#include "utilities.h"
#include <mongoc.h>

#include <vector>
#include <string>
#include <set>

using namespace std;

//template<typename T> class clsRasterData;

/*!
 * \brief Macro definitions of Raster of Watershed modeling datasets for I/O in MongoDB
 * \version 1.0
 * \author Liangjun Zhu
 */
#define MONG_GRIDFS_FN                "filename"
#define MONG_GRIDFS_WEIGHT_CELLS      "NUM_CELLS"
#define MONG_GRIDFS_WEIGHT_SITES      "NUM_SITES"
#define MONG_GRIDFS_ID                "ID"
#define MONG_GRIDFS_SUBBSN            "SUBBASIN"

class MongoGridFS;

/*!
 * \class MongoClient
 * \brief Create a MongoDB Client
 */
class MongoClient {
public:
    /*!
     * \brief Constructor
     * Get a client for a MongoDB instance using ip address and port number
     * Caution: The constructor should be used with caution, since it does
     *          not check the validation of IP address and the status of 
     *          the database. 
     *          So, `MongoClient *client = MongoClient::Init(host, port)`
     *          is highly recommended.
     */
    MongoClient(const char *host, uint16_t port);
    /*!
     * \brief Validation check before the constructor of MongoClient.
     *        1. Check IP address
     *        2. Check database status
     * \usage 
     *       MongoClient *client = MongoClient::Init(host, port)
     *       if (nullptr == client) {
     *           throw exception("MongoClient initialization failed!");
     *           // or other error handling code.
     *       }
     */
    static MongoClient* Init(const char *host, uint16_t port);
    /*!
     * \brief Destructor
     */
    ~MongoClient();

    /*!
     * \brief Get mongoc_client instance
     */
    mongoc_client_t *getConn() { return m_conn; }

    /*!
     * \brief Get existed Database instance
     * Besides, Databases are automatically created on the MongoDB server
     * upon insertion of the first document into a collection.
     * Therefore, there is no need to create a database manually.
     */
    mongoc_database_t* getDatabase(string const &dbname);

    /*!
     * \brief Get Collection instance
     */
    mongoc_collection_t* getCollection(string const &dbname, string const &collectionname);

    /*!
     * \brief Get GridFS instance
     */
    mongoc_gridfs_t *getGridFS(string const &dbname, string const &gfsname);

    /*!
    * \brief Get MongoGridFS instance
    */
    MongoGridFS *GridFS(string const &dbname, string const &gfsname);

    /*!
     * \brief Get database names
     * \return Database names, vector<string>
     */
    void getDatabaseNames(vector<string> &dbnames);

    /*!
     * \brief Get collection names in MongoDB database
     * \param[in] dbName \string database name
     */
    void getCollectionNames(string const &dbName, vector<string> &collNames);

    /*!
     * \brief Get GridFs file names in MongoDB database
     * \param[in] dbName \string database name
     * \param[in] gfs \string GridFS name
     * \return filenames vector<string>
     */
    void getGridFSFileNames(string const &dbname, string const &gfsname, vector<string>& fileExisted);

private:
    const char *m_host;
    uint16_t m_port;
    mongoc_client_t *m_conn;
};

/*!
 * \class MongoDatabase
 * \brief Create a MongoDB database instance
 */
class MongoDatabase {
public:
    /*!
     * \brief Constructor, initialized by a mongoc_database_t* pointer
     */
    explicit MongoDatabase(mongoc_database_t *db);

    /*!
     * \brief Constructor, initialized by mongodb client and database name
     */
    MongoDatabase(mongoc_client_t *conn, string const &dbname);

    /*!
     * \brief Destructor by Destroy function
     */
    ~MongoDatabase();

    /*!
      * \brief Get collection names in current database
      */
    void getCollectionNames(vector<string>& collNames);

private:
    mongoc_database_t *m_db;
    string m_dbname;
};

class MongoCollection {
public:
    /*!
    * \brief Constructor, initialized by a mongoc_collection_t* pointer
    */
    explicit MongoCollection(mongoc_collection_t* coll);
    //! Destructor
    ~MongoCollection();
    /*!
     * \brief Execute query
     */
    mongoc_cursor_t* ExecuteQuery(const bson_t* b);
    /*!
    * \brief Query the records number
    */
    int QueryRecordsCount();
private:
    mongoc_collection_t*      m_collection;
};

/*!
 * \class MongoGridFS
 * \brief Create a MongoDB GridFS instance
 */
class MongoGridFS {
public:
    /*!
     * \brief Constructor, initialized by a mongoc_gridfs_t* pointer or NULL
     */
    explicit MongoGridFS(mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Destructor by Destroy function
     */
    ~MongoGridFS();

    mongoc_gridfs_t *getGridFS() { return m_gfs; }

    /*!
     * \brief Get GridFS file by name
     */
    mongoc_gridfs_file_t *getFile(string const &gfilename, mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Remove GridFS file by name
     */
    bool removeFile(string const &gfilename, mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Get GridFS file names
     */
    void getFileNames(vector<string>&fileExisted, mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Get metadata of a given GridFS file name
     */
    bson_t *getFileMetadata(string const &gfilename, mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Get stream data of a given GridFS file name
     */
    void getStreamData(string const &gfilename, char *&databuf, size_t &datalength, 
                       mongoc_gridfs_t *gfs = NULL);

    /*!
     * \brief Write stream data to a GridFS file
     */
    void writeStreamData(string const &gfilename, char *&buf, size_t &length, 
                         const bson_t *p, mongoc_gridfs_t *gfs = NULL);

private:
    mongoc_gridfs_t *m_gfs;
};

/*!
 * \brief Get numeric value from \a bson_iter_t according to a given key
 * \param[in] iter \a bson_iter_t
 * \param[in] key 
 * \param[in] numericvalue, int, float, or double
 * \return True if succeed, otherwise false.
 */
template<typename T>
bool GetNumericFromBsonIterator(bson_iter_t *iter, T &numericvalue) {
    const bson_value_t *vv = bson_iter_value(iter);
    if (vv->value_type == BSON_TYPE_INT32) {
        numericvalue = (T) vv->value.v_int32;
    } else if (vv->value_type == BSON_TYPE_INT64) {
        numericvalue = (T) vv->value.v_int64;
    } else if (vv->value_type == BSON_TYPE_DOUBLE) {
        numericvalue = (T) vv->value.v_double;
    } else if (vv->value_type == BSON_TYPE_UTF8) {
        string tmp = vv->value.v_utf8.str;
        numericvalue = (T) atof(tmp.c_str());
    } else {
        cout << "bson iterator isn't or not contains a numeric value." << endl;
        return false;
    }
    return true;
}

/*!
 * \brief Get numeric value from \a bson_t according to a given key
 * \param[in] bmeta \a bson_t
 * \param[in] key 
 * \param[in] numericvalue, int, float, or double
 * \return True if succeed, otherwise false.
 */
template<typename T>
bool GetNumericFromBson(bson_t *bmeta, const char *key, T &numericvalue) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
        return GetNumericFromBsonIterator(&iter, numericvalue);
    } else {
        StatusMessage(("WARNING: GetNumericFromBson, Failed in get value of: " + string(key) + "\n").c_str());
        return false;
    }
}

/*!
 * \brief Get String from \a bson_iter_t
 * \param[in] bmeta \a bson_iter_t
 * \return Float value if success, or "" if failed.
 */
string GetStringFromBsonIterator(bson_iter_t *iter);

/*!
 * \brief Get String from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return Float value if success, or "" if failed.
 */
string GetStringFromBson(bson_t *bmeta, const char *key);

/*!
 * \brief Get Bool from \a bson_iter_t
 * \param[in] bmeta \a bson_iter_t
 * \return true if success, or false if failed.
 */
bool GetBoolFromBsonIterator(bson_iter_t *iter);

/*!
 * \brief Get String from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return true if success, or false if failed.
 */
bool GetBoolFromBson(bson_t *bmeta, const char *key);

/*!
 * \brief Get Datetime from \a bson_iter_t
 * \param[in] bmeta \a bson_iter_t
 * \return time_t float value if success, or -1 if failed.
 */
time_t GetDatetimeFromBsonIterator(bson_iter_t *iter);

/*!
 * \brief Get Datetime from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return time_t float value if success, or -1 if failed.
 */
time_t GetDatetimeFromBson(bson_t *bmeta, const char *key);

#endif