/*!
 * \brief Implementation of utility functions of mongoDB
 * \author Junzhi Liu, LiangJun Zhu
 * \date May 2016
 *
 * 
 */

#include "MongoUtil.h"

using namespace std;

///////////////////////////////////////////////////
////////////////  MongoClient    //////////////////
///////////////////////////////////////////////////
MongoClient::MongoClient(const char *host, int port) : m_host(host), m_port(port) {
    try {
        if (!isIPAddress(m_host)) {
            throw ModelException("MongoClient", "Connect to MongoDB",
                                 "IP address: " + string(m_host) + " is invalid, Please check!\n");
        }
        mongoc_init();
        mongoc_uri_t *uri = mongoc_uri_new_for_host_port(m_host, m_port);
        m_conn = mongoc_client_new_from_uri(uri);
        /// Check the connection to MongoDB is success or not
        bson_t *reply = bson_new();
        bson_error_t *err = NULL;
        if (!mongoc_client_get_server_status(m_conn, NULL, reply, err)) {
            throw ModelException("MongoClient", "Connect to MongoDB", "Failed to connect to MongoDB!\n");
        }
        bson_destroy(reply);
        mongoc_uri_destroy(uri);
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
    catch (exception e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

MongoClient::~MongoClient() {
    if (!m_conn) mongoc_client_destroy(m_conn);
    mongoc_cleanup();
}

void MongoClient::_database_names() {
    char **dbnames;
    unsigned i;
    bson_error_t *err = NULL;
    dbnames = mongoc_client_get_database_names(m_conn, err);
    if (!m_dbnames.empty()) {
        m_dbnames.clear();
    } /// get clear before push database names
    if (err == NULL) {
        for (i = 0; dbnames[i]; i++) {
            // cout<<dbnames[i]<<endl;
            m_dbnames.push_back(string(dbnames[i]));
        }
        bson_strfreev(dbnames);
    }
    vector<string>(m_dbnames).swap(m_dbnames);
}

vector <string> MongoClient::getDatabaseNames() {
    if (m_dbnames.empty()) this->_database_names();
    return m_dbnames;
}

vector <string> MongoClient::getCollectionNames(string const &dbName) {
    mongoc_database_t *database = this->getDatabase(dbName);
    return MongoDatabase(database).getCollectionNames();
}

mongoc_database_t *MongoClient::getDatabase(string const &dbname) {
    if (m_dbnames.empty()) this->_database_names();
    string tmpdbname = dbname;
    if (!ValueInVector(tmpdbname, m_dbnames)) {
        StatusMessage(("WARNING: Database " + tmpdbname + " is not existed and will be created!\n").c_str());
    }
    return mongoc_client_get_database(m_conn, dbname.c_str());
}

mongoc_collection_t *MongoClient::getCollection(string const &dbname, string const &collectionname) {
    try {
        mongoc_database_t *db = this->getDatabase(dbname);
        if (!mongoc_database_has_collection(db, collectionname.c_str(), NULL)) {
            throw ModelException("MongoClient", "getCollection", "Collection " + collectionname + " is not existed!\n");
        }
        mongoc_collection_t *collection = mongoc_database_get_collection(db, collectionname.c_str());
        mongoc_database_destroy(db);
        return collection;
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
}

vector <string> MongoClient::getGridFSFileNames(string const &dbname, string const &gfsname) {
    mongoc_gridfs_t *gfs = this->getGridFS(dbname, gfsname);
    return MongoGridFS(gfs).getFileNames();
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

vector <string> MongoDatabase::getCollectionNames() {
    vector <string> collNameList;
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
                vector <string> tmpList = SplitString(tmp, '.');
                vector<string>::iterator tmpIter = find(collNameList.begin(), collNameList.end(), tmpList[0]);
                if (tmpIter == collNameList.end()) {
                    collNameList.push_back(tmpList[0]);
                }
            }
        }
        vector<string>(collNameList).swap(collNameList);
        return collNameList;
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
}

vector <string> MongoGridFS::getFileNames(mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getFileNames",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        vector <string> filesExisted;
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
        return filesExisted;
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
}

void
MongoGridFS::getStreamData(string const &gfilename,
                           char *&databuf,
                           int &datalength,
                           mongoc_gridfs_t *gfs /* = NULL */) {
    try {
        if (m_gfs != NULL) gfs = m_gfs;
        if (gfs == NULL) {
            throw ModelException("MongoGridFS", "getStreamData",
                                 "mongoc_gridfs_t must be provided for MongoGridFS!\n");
        }
        mongoc_gridfs_file_t *gfile = this->getFile(gfilename, gfs);
        size_t length = (size_t) mongoc_gridfs_file_get_length(gfile);
        datalength = length;
        databuf = (char *) malloc(length);
        mongoc_iovec_t iov;
        iov.iov_base = databuf;
        iov.iov_len = length;
        mongoc_stream_t *stream;
        stream = mongoc_stream_gridfs_new(gfile);
        mongoc_stream_readv(stream, &iov, 1, -1, 0);
        mongoc_gridfs_file_destroy(gfile);
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
}

void MongoGridFS::writeStreamData(string const &gfilename, char *&buf, int &length, const bson_t *p,
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
        ovec.iov_len = (size_t) length;
        ssize_t writesize = mongoc_gridfs_file_writev(gfile, &ovec, 1, 0);
        if (writesize == -1) {
            throw ModelException("MongoUtil", "writeStreamData", "Failed to write a gridfs file!\n");
        }
        if (!mongoc_gridfs_file_save(gfile)) {
            throw ModelException("MongoUtil", "writeStreamData", "Failed when save modifications to GridFS file!\n");
        }
        mongoc_gridfs_file_destroy(gfile);
    }
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
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
    catch (ModelException e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
}
