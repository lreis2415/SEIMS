/*!
 * \brief Utility functions of MongoDB
 *        Currently, mongo-c-driver 1.3.5 or later is supported.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * \note No exceptions will be thrown.
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 * \changelog 2017-12-02 - lj - Add unittest based on gtest/gmock.\n
 *            2018-05-02 - lj - Make part of CCGL.\n
 */
#ifndef CCGL_DB_MONGOC_H
#define CCGL_DB_MONGOC_H
#ifdef USE_MONGODB

#include <vector>
#include <iostream>

#include <mongoc.h>

#include "basic.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;

namespace ccgl {
/*!
 * \namespace db_mongoc
 * \brief Utility functions of MongoDB
 */
namespace db_mongoc {
class MongoGridFs;

/*!
 * \class MongoClient
 * \brief Create a MongoDB Client
 */
class MongoClient: NotCopyable {
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
    MongoClient(const char* host, vuint16_t port);
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
    static MongoClient* Init(const char* host, vuint16_t port);
    /*!
     * \brief Destructor
     */
    ~MongoClient();

    /*!
     * \brief Get mongoc_client instance
     */
    mongoc_client_t* GetConn() { return conn_; }

    /*!
     * \brief Get existed Database instance
     * Besides, Databases are automatically created on the MongoDB server
     * upon insertion of the first document into a collection.
     * Therefore, there is no need to create a database manually.
     */
    mongoc_database_t* GetDatabase(string const& dbname);

    /*!
     * \brief Get Collection instance
     */
    mongoc_collection_t* GetCollection(string const& dbname, string const& collectionname);

    /*!
     * \brief Get GridFS instance
     */
    mongoc_gridfs_t* GetGridFs(string const& dbname, string const& gfsname);

    /*!
    * \brief Get MongoGridFs instance
    */
    MongoGridFs* GridFs(string const& dbname, string const& gfsname);

    /*!
     * \brief Get database names
     * \return Database names, vector<string>
     */
    void GetDatabaseNames(vector<string>& dbnames);

    /*!
     * \brief Get collection names in MongoDB database
     * \param[in] dbname \string database name
     * \param[out] collnames Collection names in the database
     */
    void GetCollectionNames(string const& dbname, vector<string>& collnames);

    /*!
     * \brief Get GridFs file names in MongoDB database
     * \param[in] dbname \string database name
     * \param[in] gfsname \string GridFS name
     * \param[out] gfs_exists Existed GridFS file names
     */
    void GetGridFsFileNames(string const& dbname, string const& gfsname, vector<string>& gfs_exists);

private:
    const char* host_;
    vuint16_t port_;
    mongoc_client_t* conn_;
};

/*!
 * \class MongoDatabase
 * \brief Create a MongoDB database instance
 */
class MongoDatabase: NotCopyable {
public:
    /*!
     * \brief Constructor, initialized by a mongoc_database_t* pointer
     */
    explicit MongoDatabase(mongoc_database_t* db);

    /*!
     * \brief Constructor, initialized by mongodb client and database name
     */
    MongoDatabase(mongoc_client_t* conn, string const& dbname);

    /*!
     * \brief Destructor by Destroy function
     */
    ~MongoDatabase();

    /*!
      * \brief Get collection names in current database
      */
    void GetCollectionNames(vector<string>& collnames);

private:
    mongoc_database_t* db_;
    string dbname_;
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
    mongoc_collection_t* collection_;
};

/*!
 * \class MongoGridFs
 * \brief Create a MongoDB GridFS instance
 */
class MongoGridFs {
public:
    /*!
     * \brief Constructor, initialized by a mongoc_gridfs_t* pointer or NULL
     */
    explicit MongoGridFs(mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Destructor by Destroy function
     */
    ~MongoGridFs();

    mongoc_gridfs_t* GetGridFs() { return gfs_; }

    /*!
     * \brief Get GridFS file by name
     */
    mongoc_gridfs_file_t* GetFile(string const& gfilename, mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Remove GridFS file by name
     */
    bool RemoveFile(string const& gfilename, mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Get GridFS file names
     */
    void GetFileNames(vector<string>& files_existed, mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Get metadata of a given GridFS file name
     */
    bson_t* GetFileMetadata(string const& gfilename, mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Get stream data of a given GridFS file name
     */
    void GetStreamData(string const& gfilename, char*& databuf, size_t& datalength,
                       mongoc_gridfs_t* gfs = NULL);

    /*!
     * \brief Write stream data to a GridFS file
     */
    void WriteStreamData(const string& gfilename, char*& buf, size_t length,
                         const bson_t* p, mongoc_gridfs_t* gfs = NULL);

private:
    mongoc_gridfs_t* gfs_;
};

/*!
 * \brief Get numeric value from \a bson_iter_t according to a given key
 * \param[in] iter \a bson_iter_t
 * \param[out] numericvalue \int, \float, or \double
 * \return True if succeed, otherwise false.
 */
template <typename T>
bool GetNumericFromBsonIterator(bson_iter_t* iter, T& numericvalue) {
    const bson_value_t* vv = bson_iter_value(iter);
    if (vv->value_type == BSON_TYPE_INT32) {
        numericvalue = static_cast<T>(vv->value.v_int32);
    } else if (vv->value_type == BSON_TYPE_INT64) {
        numericvalue = static_cast<T>(vv->value.v_int64);
    } else if (vv->value_type == BSON_TYPE_DOUBLE) {
        numericvalue = static_cast<T>(vv->value.v_double);
    } else if (vv->value_type == BSON_TYPE_UTF8) {
        string tmp = vv->value.v_utf8.str;
        if (tmp.find_first_of("0123456789") == string::npos) {
            cout << "bson iterator isn't or not contains a numeric value." << endl;
            return false;
        }
        char* end = nullptr;
        numericvalue = static_cast<T>(strtod(tmp.c_str(), &end));
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
 * \param[out] numericvalue \int, \float, or \double
 * \return True if succeed, otherwise false.
 */
template <typename T>
bool GetNumericFromBson(bson_t* bmeta, const char* key, T& numericvalue) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
        return GetNumericFromBsonIterator(&iter, numericvalue);
    }
    StatusMessage(("WARNING: GetNumericFromBson, Failed in get value of: " + string(key) + "\n").c_str());
    return false;
}

/*!
 * \brief Get String from \a bson_iter_t
 * \param[in] iter \a bson_iter_t
 * \return Float value if success, or "" if failed.
 */
string GetStringFromBsonIterator(bson_iter_t* iter);

/*!
 * \brief Get String from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return Float value if success, or "" if failed.
 */
string GetStringFromBson(bson_t* bmeta, const char* key);

/*!
 * \brief Get Bool from \a bson_iter_t
 * \param[in] iter \a bson_iter_t
 * \return true if success, or false if failed.
 */
bool GetBoolFromBsonIterator(bson_iter_t* iter);

/*!
 * \brief Get String from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return true if success, or false if failed.
 */
bool GetBoolFromBson(bson_t* bmeta, const char* key);

/*!
 * \brief Get Datetime from \a bson_iter_t
 * \param[in] iter \a bson_iter_t
 * \return time_t float value if success, or -1 if failed.
 */
time_t GetDatetimeFromBsonIterator(bson_iter_t* iter);

/*!
 * \brief Get Datetime from \a bson_t
 * \param[in] bmeta \a bson_t
 * \param[in] key
 * \return time_t float value if success, or -1 if failed.
 */
time_t GetDatetimeFromBson(bson_t* bmeta, const char* key);
} /* namespace: db_mongoc */
} /* namespace: ccgl */

#endif /* USE_MONGODB */
#endif /* CCGL_DB_MONGOC_H */
