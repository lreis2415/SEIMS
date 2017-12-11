#include "MongoUtil.h"

using namespace std;

///////////////////////////////////////////////////
////////////////  MongoClient    //////////////////
///////////////////////////////////////////////////
MongoClient::MongoClient(const char *host, uint16_t port) : m_host(host), m_port(port) {
    assert(isIPAddress(host));
    assert(port > 0);
    mongoc_init();
    mongoc_uri_t *uri = mongoc_uri_new_for_host_port(m_host, m_port);
    m_conn = mongoc_client_new_from_uri(uri);

    mongoc_uri_destroy(uri);
}

MongoClient* MongoClient::Init(const char *host, uint16_t port) {
    if (!isIPAddress(host)) {
        cout << "IP address: " + string(host) + " is invalid, Please check!" << endl;
        return nullptr;
    }
    mongoc_init();
    mongoc_uri_t *uri = mongoc_uri_new_for_host_port(host, port);
    mongoc_client_t *conn = mongoc_client_new_from_uri(uri);
    /// Check the connection to MongoDB is success or not
    bson_t *reply = bson_new();
    bson_error_t *err = NULL;
    if (!mongoc_client_get_server_status(conn, NULL, reply, err)) {
        cout << "Failed to connect to MongoDB!" << endl;
        return nullptr;
    }
    bson_destroy(reply);
    mongoc_uri_destroy(uri);
    mongoc_client_destroy(conn);

    return new MongoClient(host, port);
}
MongoClient::~MongoClient() {
    StatusMessage("Releasing MongoClient ...");
    if (m_conn) {
        StatusMessage("Close database connection ...");
        mongoc_client_destroy(m_conn);
    }
    mongoc_cleanup();
}

void MongoClient::getDatabaseNames(vector<string> &dbnames) {
    char **dbnames_char = NULL;
    unsigned i;
    bson_error_t *err = NULL;
    dbnames_char = mongoc_client_get_database_names(m_conn, err);
    if (!dbnames.empty()) {
        dbnames.clear();
    } /// get clear before push database names
    if (err == NULL) {
        for (i = 0; dbnames_char[i]; i++) {
            // cout<<dbnames[i]<<endl;
            dbnames.push_back(string(dbnames_char[i]));
        }
        bson_strfreev(dbnames_char);
    }
    vector<string>(dbnames).swap(dbnames);
}

void MongoClient::getCollectionNames(string const &dbName, vector<string> &collNames) {
    MongoDatabase(this->getDatabase(dbName)).getCollectionNames(collNames);
}

mongoc_database_t* MongoClient::getDatabase(string const &dbname) {
    // Get Database or create if not existed
    return mongoc_client_get_database(m_conn, dbname.c_str());
}

mongoc_collection_t* MongoClient::getCollection(string const &dbname, string const &collectionname) {
    mongoc_database_t* db = mongoc_client_get_database(m_conn, dbname.c_str());
    if (!mongoc_database_has_collection(db, collectionname.c_str(), NULL)) {
        cout << "MongoClient::getCollection, Collection " << collectionname << " is not existed!" << endl;
        return NULL;
    }
    mongoc_collection_t *collection = mongoc_database_get_collection(db, collectionname.c_str());
    mongoc_database_destroy(db);
    return collection;
}

mongoc_gridfs_t *MongoClient::getGridFS(string const &dbname, string const &gfsname) {
    bson_error_t err;
    mongoc_gridfs_t *gfs;
    try {
        gfs = mongoc_client_get_gridfs(m_conn, dbname.c_str(), gfsname.c_str(), &err);
        if (gfs == NULL) {
            throw ModelException("MongoClient", "getGridFS", "Failed to connect to " + gfsname + " GridFS!\n");
        }
        return gfs;
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return NULL;
    }
}

void MongoClient::getGridFSFileNames(string const &dbname, string const &gfsname, vector<string>&fileExists) {
    mongoc_gridfs_t *gfs = this->getGridFS(dbname, gfsname);
    MongoGridFS(gfs).getFileNames(fileExists);
}

///////////////////////////////////////////////////
////////////////  MongoDatabase  //////////////////
///////////////////////////////////////////////////
MongoDatabase::MongoDatabase(mongoc_database_t *db) : m_db(db) {
    m_dbname = string(mongoc_database_get_name(m_db));
}

MongoDatabase::MongoDatabase(mongoc_client_t *conn, string const &dbname) {
    m_dbname = dbname;
    m_db = mongoc_client_get_database(conn, m_dbname.c_str());
}

MongoDatabase::~MongoDatabase() {
    mongoc_database_destroy(m_db);
}

void MongoDatabase::getCollectionNames(vector<string>& collNameList) {
    try {
        mongoc_cursor_t *cursor;
        bson_error_t *err = NULL;
        const bson_t *doc;
        cursor = mongoc_database_find_collections(m_db, NULL, err);
        if (err != NULL) {
            throw ModelException("MongoClient", "get_collection_names",
                                 "There is no collections in database: " + m_dbname + ".\n");
        }
        bson_iter_t iter;
        while (mongoc_cursor_next(cursor, &doc)) {
            if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "name")) {
                string tmp = GetStringFromBsonIterator(&iter);
                vector<string> tmpList = SplitString(tmp, '.');
                vector<string>::iterator tmpIter = find(collNameList.begin(), collNameList.end(), tmpList[0]);
                if (tmpIter == collNameList.end()) {
                    collNameList.push_back(tmpList[0]);
                }
            }
        }
        vector<string>(collNameList).swap(collNameList);
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        collNameList.clear();
    }
}
///////////////////////////////////////////////////
////////////////  MongoCollection  ////////////////
///////////////////////////////////////////////////
MongoCollection::MongoCollection(mongoc_collection_t* coll) : m_collection(coll) {

}
MongoCollection::~MongoCollection(void) {
    mongoc_collection_destroy(m_collection);
}
mongoc_cursor_t* MongoCollection::ExecuteQuery(const bson_t* b) {
    // printf("%s\n", bson_as_json(b, NULL));
    // TODO: mongoc_collection_find should be deprecated, however, mongoc_collection_find_with_opts
    //       may not work in my Windows 10 both by MSVC and MINGW64.
    //       So, remove `&& !defined(windows)` when this bug fixed. LJ
#if MONGOC_CHECK_VERSION(1, 5, 0) && !defined(windows)
    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(m_collection, b, NULL, NULL);
#else
    mongoc_cursor_t* cursor = mongoc_collection_find(m_collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
#endif /* MONGOC_CHECK_VERSION */
    return cursor;
}

int MongoCollection::QueryRecordsCount(void) {
    const bson_t *qCount = bson_new();
    bson_error_t *err = NULL;
    int count = (int)mongoc_collection_count(m_collection, MONGOC_QUERY_NONE, qCount, 0, 0, NULL, err);
    if (err != NULL || count < 0) {
        cout << "ERROR: Failed to get document number of collection!" << endl;
        return -1;
    }
    return count;
}

///////////////////////////////////////////////////
////////////////  MongoGridFS  ////////////////////
///////////////////////////////////////////////////
MongoGridFS::MongoGridFS(mongoc_gridfs_t *gfs /* = NULL */) {
    m_gfs = gfs;
}

MongoGridFS::~MongoGridFS() {
    if (m_gfs != NULL) {
        mongoc_gridfs_destroy(m_gfs);
    }
}

mongoc_gridfs_file_t *MongoGridFS::getFile(string const &gfilename, mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getFile",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        mongoc_gridfs_file_t *gfile = NULL;
        bson_error_t err;
        gfile = mongoc_gridfs_find_one_by_filename(gfs, gfilename.c_str(), &err);
        if (gfile == NULL) {
            throw ModelException("MongoGridFS", "getFile",
                                 "The file " + gfilename + " does not exist.");
        }
        return gfile;
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return NULL;
    }
}

bool MongoGridFS::removeFile(string const &gfilename, mongoc_gridfs_t *gfs /* = NULL */) {
    bool removedone = true;
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getFile",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        bson_error_t *err = NULL;
        removedone = mongoc_gridfs_remove_by_filename(gfs, gfilename.c_str(), err);
        if (err != NULL || !removedone) removedone = false;
        return removedone;
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return false;
    }
}

void MongoGridFS::getFileNames(vector<string>&filesExisted, mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getFileNames",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        //vector<string> filesExisted;
        bson_t *query = bson_new();
        bson_init(query);
        mongoc_gridfs_file_list_t *glist = mongoc_gridfs_find_with_opts(gfs, query, NULL);
        mongoc_gridfs_file_t *file;
        while ((file = mongoc_gridfs_file_list_next(glist))) {
            filesExisted.push_back(string(mongoc_gridfs_file_get_filename(file)));
            mongoc_gridfs_file_destroy(file);
        }
        mongoc_gridfs_file_list_destroy(glist);
        vector<string>(filesExisted).swap(filesExisted);
        //return filesExisted;
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return;
    }
}

bson_t *MongoGridFS::getFileMetadata(string const &gfilename, mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getFileMetadata",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        mongoc_gridfs_file_t *gfile = this->getFile(gfilename, gfs);
        const bson_t *bmata = mongoc_gridfs_file_get_metadata(gfile);
        bson_t *mata = bson_copy(bmata);
        mongoc_gridfs_file_destroy(gfile);
        return mata;
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return NULL;
    }
}

void
MongoGridFS::getStreamData(string const &gfilename,
                           char *&databuf,
                           size_t &datalength,
                           mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getStreamData",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        mongoc_gridfs_file_t *gfile = this->getFile(gfilename, gfs);
        datalength = (size_t)mongoc_gridfs_file_get_length(gfile);
        databuf = (char *)malloc(datalength);
        mongoc_iovec_t iov;
        iov.iov_base = databuf;
        iov.iov_len = datalength;
        mongoc_stream_t *stream;
        stream = mongoc_stream_gridfs_new(gfile);
        mongoc_stream_readv(stream, &iov, 1, -1, 0);
        mongoc_gridfs_file_destroy(gfile);
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return;
    }
}

void MongoGridFS::writeStreamData(string const &gfilename, char *&buf, size_t &length, const bson_t *p,
                                  mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "writeStreamData",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }

        mongoc_gridfs_file_t *gfile = NULL;
        mongoc_gridfs_file_opt_t gopt = {0};
        gopt.filename = gfilename.c_str();
        gopt.content_type = "NumericStream";
        gopt.metadata = p;
        gfile = mongoc_gridfs_create_file(gfs, &gopt);
        mongoc_iovec_t ovec;
        ovec.iov_base = buf;
        ovec.iov_len = length;
        ssize_t writesize = mongoc_gridfs_file_writev(gfile, &ovec, 1, 0);
        if (writesize == -1) {
            throw ModelException("MongoUtil", "writeStreamData", "Failed to write a gridfs file!\n");
        }
        if (!mongoc_gridfs_file_save(gfile)) {
            throw ModelException("MongoUtil", "writeStreamData", "Failed when save modifications to GridFS file!\n");
        }
        mongoc_gridfs_file_destroy(gfile);
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return;
    }
}

///////////////////////////////////////////////////
/////////  bson related utilities   ///////////////
///////////////////////////////////////////////////
string GetStringFromBsonIterator(bson_iter_t *iter) {
    try {
        const bson_value_t *vv = bson_iter_value(iter);
        if (vv->value_type == BSON_TYPE_UTF8) {
            return vv->value.v_utf8.str;
        } else if (vv->value_type == BSON_TYPE_INT32) {
            return ValueToString(vv->value.v_int32);
        } else if (vv->value_type == BSON_TYPE_INT64) {
            return ValueToString(vv->value.v_int64);
        } else if (vv->value_type == BSON_TYPE_DOUBLE) {
            return ValueToString(vv->value.v_double);
        } else {
            throw ModelException("MongoUtil", "GetStringFromBson",
                                 "bson iterator did not contain or can not convert to string.\n");
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return "";
    }
}

string GetStringFromBson(bson_t *bmeta, const char *key) {
    try {
        bson_iter_t iter;
        if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
            return GetStringFromBsonIterator(&iter);
        } else {
            throw ModelException("MongoUtil", "GetStringFromBson",
                                 "Failed in get value of: " + string(key) + "\n");
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return "";
    }
}

bool GetBoolFromBsonIterator(bson_iter_t *iter) {
    try {
        const bson_value_t *vv = bson_iter_value(iter);
        float fltvalue;
        if (vv->value_type == BSON_TYPE_INT32) {
            fltvalue = (float) vv->value.v_int32;
        } else if (vv->value_type == BSON_TYPE_INT64) {
            fltvalue = (float) vv->value.v_int64;
        } else if (vv->value_type == BSON_TYPE_DOUBLE) {
            fltvalue = (float) vv->value.v_double;
        } else if (vv->value_type == BSON_TYPE_UTF8) {
            string tmp = vv->value.v_utf8.str;
            if (StringMatch(tmp.c_str(), "TRUE")) {
                return true;
            } else {
                return false;
            }
        } else {
            throw ModelException("MongoUtil", "GetBoolFromBsonIterator", "Failed in get Boolean value.\n");
        }
        if (FloatEqual(fltvalue, 0.f)) {
            return false;
        } else {
            return true;
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return false;
    }
}

bool GetBoolFromBson(bson_t *bmeta, const char *key) {
    try {
        bson_iter_t iter;
        if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
            return GetBoolFromBsonIterator(&iter);
        } else {
            throw ModelException("MongoUtil", "GetBoolFromBson",
                                 "Failed in get boolean value of: " + string(key) + "\n");
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return false;
    }
}

time_t GetDatetimeFromBsonIterator(bson_iter_t *iter) {
    try {
        const bson_value_t *vv = bson_iter_value(iter);
        if (vv->value_type == BSON_TYPE_DATE_TIME) {
            return (time_t) vv->value.v_datetime;
        } else if (vv->value_type == BSON_TYPE_UTF8) {
            string tmpTimeStr = vv->value.v_utf8.str;
            if (tmpTimeStr.size() > 12) {
                return ConvertToTime2(tmpTimeStr, "%4d-%2d-%2d %2d:%2d:%2d", true);
            } else {
                return ConvertToTime2(tmpTimeStr, "%4d-%2d-%2d", false);
            }
        } else {
            throw ModelException("MongoUtil", "GetDatetimeFromBsonIterator", "Failed in get Date Time value.\n");
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return -1;
    }
}

time_t GetDatetimeFromBson(bson_t *bmeta, const char *key) {
    try {
        bson_iter_t iter;
        if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
            return GetDatetimeFromBsonIterator(&iter);
        } else {
            throw ModelException("MongoUtil", "GetDatetimeFromBson",
                                 "Failed in get Datetime value of: " + string(key) + "\n");
        }
    }
    catch (ModelException& e) {
        cout << e.toString() << endl;
        return -1;
    }
}
