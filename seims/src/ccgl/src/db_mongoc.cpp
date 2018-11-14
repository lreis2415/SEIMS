#include "db_mongoc.h"

#include <cassert>
#include "basic.h"
#include "utils_string.h"
#include "utils_math.h"
#include "utils_time.h"

using std::cout;
using std::endl;

#ifdef USE_MONGODB
namespace ccgl {
using namespace utils_string;
using namespace utils_time;
namespace db_mongoc {
///////////////////////////////////////////////////
////////////////  MongoClient    //////////////////
///////////////////////////////////////////////////
MongoClient::MongoClient(const char* host, vuint16_t port) : host_(host), port_(port) {
    assert(IsIpAddress(host));
    mongoc_init();
    mongoc_uri_t* uri = mongoc_uri_new_for_host_port(host_, port_);
    conn_ = mongoc_client_new_from_uri(uri);

    mongoc_uri_destroy(uri);
}

MongoClient* MongoClient::Init(const char* host, const vuint16_t port) {
    if (!IsIpAddress(host)) {
        cout << "IP address: " + string(host) + " is invalid, Please check!" << endl;
        return nullptr;
    }
    mongoc_init();
    mongoc_uri_t* uri = mongoc_uri_new_for_host_port(host, port);
    mongoc_client_t* conn = mongoc_client_new_from_uri(uri);
    /// Check the connection to MongoDB is success or not
    bson_t* reply = bson_new();
    bson_error_t* err = NULL;
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
    if (conn_) {
        /// Check the connection to MongoDB is success or not
        bson_t* reply = bson_new();
        bson_error_t* err = NULL;
        if (mongoc_client_get_server_status(conn_, NULL, reply, err) && err != NULL) {
            mongoc_client_destroy(conn_);
        }
        bson_destroy(reply);
    }
    mongoc_cleanup();
}

void MongoClient::GetDatabaseNames(vector<string>& dbnames) {
    char** dbnames_char = NULL;
    bson_error_t* err = NULL;
    dbnames_char = mongoc_client_get_database_names(conn_, err);
    if (!dbnames.empty()) {
        dbnames.clear();
    } /// get clear before push database names
    if (err == NULL) {
        for (vuint i = 0; dbnames_char[i]; i++) {
            // cout<<dbnames[i]<<endl;
            dbnames.emplace_back(string(dbnames_char[i]));
        }
        bson_strfreev(dbnames_char);
    }
    vector<string>(dbnames).swap(dbnames);
}

void MongoClient::GetCollectionNames(string const& dbname, vector<string>& collnames) {
    MongoDatabase(this->GetDatabase(dbname)).GetCollectionNames(collnames);
}

mongoc_database_t* MongoClient::GetDatabase(string const& dbname) {
    // Get Database or create if not existed
    return mongoc_client_get_database(conn_, dbname.c_str());
}

mongoc_collection_t* MongoClient::GetCollection(string const& dbname, string const& collectionname) {
    mongoc_database_t* db = mongoc_client_get_database(conn_, dbname.c_str());
    if (!mongoc_database_has_collection(db, collectionname.c_str(), NULL)) {
        cout << "MongoClient::GetCollection, Collection " << collectionname << " is not existed!" << endl;
        return NULL;
    }
    mongoc_collection_t* collection = mongoc_database_get_collection(db, collectionname.c_str());
    mongoc_database_destroy(db);
    return collection;
}

mongoc_gridfs_t* MongoClient::GetGridFs(string const& dbname, string const& gfsname) {
    bson_error_t err;
    mongoc_gridfs_t* gfs = mongoc_client_get_gridfs(conn_, dbname.c_str(), gfsname.c_str(), &err);
    if (gfs == NULL) {
        cout << "Failed to connect to " + gfsname + " GridFS!" << endl;
        return NULL;
    }
    return gfs;
}

MongoGridFs* MongoClient::GridFs(string const& dbname, string const& gfsname) {
    return new MongoGridFs(GetGridFs(dbname, gfsname));
}

void MongoClient::GetGridFsFileNames(string const& dbname, string const& gfsname, vector<string>& gfs_exists) {
    mongoc_gridfs_t* gfs = GetGridFs(dbname, gfsname);
    MongoGridFs(gfs).GetFileNames(gfs_exists);
}

///////////////////////////////////////////////////
////////////////  MongoDatabase  //////////////////
///////////////////////////////////////////////////
MongoDatabase::MongoDatabase(mongoc_database_t* db) : db_(db) {
    dbname_ = string(mongoc_database_get_name(db_));
}

MongoDatabase::MongoDatabase(mongoc_client_t* conn, string const& dbname) : dbname_(dbname) {
    db_ = mongoc_client_get_database(conn, dbname_.c_str());
}

MongoDatabase::~MongoDatabase() {
    mongoc_database_destroy(db_);
}

void MongoDatabase::GetCollectionNames(vector<string>& collnames) {
    bson_error_t* err = NULL;
    const bson_t* doc;
    mongoc_cursor_t* cursor = mongoc_database_find_collections(db_, NULL, err);
    if (err != NULL) {
        cout << "There is no collections in database: " << dbname_ << endl;
        collnames.clear();
    }
    bson_iter_t iter;
    while (mongoc_cursor_next(cursor, &doc)) {
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "name")) {
            string tmp = GetStringFromBsonIterator(&iter);
            vector<string> tmp_list = SplitString(tmp, '.');
            auto tmp_iter = find(collnames.begin(), collnames.end(), tmp_list[0]);
            if (tmp_iter == collnames.end()) {
                collnames.emplace_back(tmp_list[0]);
            }
        }
    }
    vector<string>(collnames).swap(collnames);
}

///////////////////////////////////////////////////
////////////////  MongoCollection  ////////////////
///////////////////////////////////////////////////
MongoCollection::MongoCollection(mongoc_collection_t* coll) : collection_(coll) {

}

MongoCollection::~MongoCollection() {
    mongoc_collection_destroy(collection_);
}

mongoc_cursor_t* MongoCollection::ExecuteQuery(const bson_t* b) {
    // printf("%s\n", bson_as_json(b, NULL));
    // TODO: mongoc_collection_find should be deprecated, however, mongoc_collection_find_with_opts
    //       may not work in my Windows 10 both by MSVC and MINGW64.
    //       So, remove `&& !defined(windows)` when this bug fixed. LJ
    //       Upd 12/13/2017 The new method also failed in our linux cluster (redhat 6.2 and Intel C++ 12.1)
    //                      So, I will uncomment these code later.
    //#if MONGOC_CHECK_VERSION(1, 5, 0) && !defined(windows)
    //    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(collection_, b, NULL, NULL);
    //#else
    mongoc_cursor_t* cursor = mongoc_collection_find(collection_, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
    //#endif /* MONGOC_CHECK_VERSION */
    return cursor;
}

int MongoCollection::QueryRecordsCount() {
    const bson_t* q_count = bson_new();
    bson_error_t* err = NULL;
    int count = CVT_INT(mongoc_collection_count(collection_, MONGOC_QUERY_NONE, q_count, 0, 0, NULL, err));
    if (err != NULL || count < 0) {
        cout << "ERROR: Failed to get document number of collection!" << endl;
        return -1;
    }
    return count;
}

///////////////////////////////////////////////////
////////////////  MongoGridFs  ////////////////////
///////////////////////////////////////////////////
MongoGridFs::MongoGridFs(mongoc_gridfs_t* gfs /* = NULL */) : gfs_(gfs) {
}

MongoGridFs::~MongoGridFs() {
    if (gfs_ != NULL) {
        mongoc_gridfs_destroy(gfs_);
    }
}

mongoc_gridfs_file_t* MongoGridFs::GetFile(string const& gfilename, mongoc_gridfs_t* gfs /* = NULL */,
                                           STRING_MAP opts /* = STRING_MAP() */) {
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
        return NULL;
    }
    mongoc_gridfs_file_t* gfile = NULL;
    bson_error_t err;
    bson_t filter = BSON_INITIALIZER;
    BSON_APPEND_UTF8(&filter, "filename", gfilename.c_str());
    AppendStringOptionsToBson(&filter, opts);
    // Replace `mongoc_gridfs_find_one_by_filename` by `mongoc_gridfs_find_one_with_opts`
    int count = 0;
    while (count < 10) {
        gfile = mongoc_gridfs_find_one_with_opts(gfs, &filter, NULL, &err);
        if (gfile == NULL) {
            SleepMs(1); // Sleep for one millisecond, in case of network blocking. By lj.
            count++;
        } else { break; }
    }
    if (gfile == NULL) {
        cout << "The file " << gfilename << " does not exist." << endl;
        return NULL;
    }
    bson_destroy(&filter);
    return gfile;
}

bool MongoGridFs::RemoveFile(string const& gfilename, mongoc_gridfs_t* gfs /* = NULL */) {
    bool removedone = true;
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
        return false;
    }
    bson_error_t* err = NULL;
    removedone = mongoc_gridfs_remove_by_filename(gfs, gfilename.c_str(), err);
    if (err != NULL || !removedone) removedone = false;
    return removedone;
}

void MongoGridFs::GetFileNames(vector<string>& files_existed, mongoc_gridfs_t* gfs /* = NULL */) {
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
    }
    //vector<string> filesExisted;
    bson_t* query = bson_new();
    bson_init(query);
    mongoc_gridfs_file_list_t* glist = mongoc_gridfs_find_with_opts(gfs, query, NULL);
    mongoc_gridfs_file_t* file;
    while ((file = mongoc_gridfs_file_list_next(glist))) {
        files_existed.emplace_back(string(mongoc_gridfs_file_get_filename(file)));
        mongoc_gridfs_file_destroy(file);
    }
    mongoc_gridfs_file_list_destroy(glist);
    vector<string>(files_existed).swap(files_existed);
}

bson_t* MongoGridFs::GetFileMetadata(string const& gfilename, mongoc_gridfs_t* gfs /* = NULL */,
                                     STRING_MAP opts /* = STRING_MAP() */) {
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
        return NULL;
    }
    mongoc_gridfs_file_t* gfile = GetFile(gfilename, gfs, opts);
    if (gfile == NULL) {
        cout << gfilename << " is not existed or get file timed out!" << endl;
        return NULL;
    }
    const bson_t* bmata = mongoc_gridfs_file_get_metadata(gfile);
    bson_t* mata = bson_copy(bmata);
    mongoc_gridfs_file_destroy(gfile);
    return mata;
}

void MongoGridFs::GetStreamData(string const& gfilename, char*& databuf,
                                size_t& datalength, mongoc_gridfs_t* gfs /* = NULL */,
                                STRING_MAP opts /* = STRING_MAP() */) {
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
        return;
    }
    mongoc_gridfs_file_t* gfile = GetFile(gfilename, gfs, opts);
    if (gfile == NULL) {
        databuf = NULL;
        cout << gfilename << " is not existed or get file timed out!" << endl;
        return;
    }
    datalength = mongoc_gridfs_file_get_length(gfile);
    databuf = static_cast<char *>(malloc(datalength));
    mongoc_iovec_t iov;
    iov.iov_base = databuf;
    iov.iov_len = static_cast<u_long>(datalength);
    mongoc_stream_t* stream = mongoc_stream_gridfs_new(gfile);
    mongoc_stream_readv(stream, &iov, 1, -1, 0);
    mongoc_gridfs_file_destroy(gfile);
}

void MongoGridFs::WriteStreamData(const string& gfilename, char*& buf, const size_t length, const bson_t* p,
                                  mongoc_gridfs_t* gfs /* = NULL */) {
    if (gfs_ != NULL) gfs = gfs_;
    if (gfs == NULL) {
        cout << "mongoc_gridfs_t must be provided for MongoGridFs!" << endl;
        return;
    }
    mongoc_gridfs_file_t* gfile = NULL;
    mongoc_gridfs_file_opt_t gopt = {0};
    gopt.filename = gfilename.c_str();
    gopt.content_type = "NumericStream";
    gopt.metadata = p;
    gfile = mongoc_gridfs_create_file(gfs, &gopt);
    mongoc_iovec_t ovec;
    ovec.iov_base = buf;
    ovec.iov_len = static_cast<u_long>(length);
    ssize_t writesize = mongoc_gridfs_file_writev(gfile, &ovec, 1, 0);
    if (writesize == -1) {
        cout << "Failed to write a gridfs file!" << endl;
    }
    if (!mongoc_gridfs_file_save(gfile)) {
        cout << "Failed when save modifications to GridFS file!" << endl;
    }
    mongoc_gridfs_file_destroy(gfile);
}

///////////////////////////////////////////////////
/////////  bson related utilities   ///////////////
///////////////////////////////////////////////////

void AppendStringOptionsToBson(bson_t* bson_opts, STRING_MAP& opts) {
    if (!opts.empty()) {
        for (auto iter = opts.begin(); iter != opts.end(); ++iter) {
            string meta_field = "metadata." + iter->first;
            bool is_dbl = false;
            double dbl_value = IsDouble(iter->second, is_dbl);
            if (StringMatch("", iter->second) || !is_dbl) {
                BSON_APPEND_UTF8(bson_opts, meta_field.c_str(), iter->second.c_str());
            }
            else {
                if (std::fmod(dbl_value, 1.) == 0) {
                    BSON_APPEND_INT32(bson_opts, meta_field.c_str(), CVT_INT(dbl_value));
                } else {
                    BSON_APPEND_DOUBLE(bson_opts, meta_field.c_str(), dbl_value);
                }
            }
        }
    }
}
string GetStringFromBsonIterator(bson_iter_t* iter) {
    const bson_value_t* vv = bson_iter_value(iter);
    if (vv->value_type == BSON_TYPE_UTF8) {
        return vv->value.v_utf8.str;
    }
    if (vv->value_type == BSON_TYPE_INT32) {
        return ValueToString(vv->value.v_int32);
    }
    if (vv->value_type == BSON_TYPE_INT64) {
        return ValueToString(vv->value.v_int64);
    }
    if (vv->value_type == BSON_TYPE_DOUBLE) {
        return ValueToString(vv->value.v_double);
    }
    cout << "bson iterator did not contain or can not convert to string." << endl;
    return "";
}

string GetStringFromBson(bson_t* bmeta, const char* key) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
        return GetStringFromBsonIterator(&iter);
    }
    cout << "Failed in get value of: " << key << endl;
    return "";
}

bool GetBoolFromBsonIterator(bson_iter_t* iter) {
    const bson_value_t* vv = bson_iter_value(iter);
    float fltvalue;
    if (vv->value_type == BSON_TYPE_INT32) {
        fltvalue = CVT_FLT(vv->value.v_int32);
    } else if (vv->value_type == BSON_TYPE_INT64) {
        fltvalue = CVT_FLT(vv->value.v_int64);
    } else if (vv->value_type == BSON_TYPE_DOUBLE) {
        fltvalue = CVT_FLT(vv->value.v_double);
    } else if (vv->value_type == BSON_TYPE_UTF8) {
        string tmp = vv->value.v_utf8.str;
        return StringMatch(tmp.c_str(), "TRUE");
    } else {
        cout << "Failed in get Boolean value." << endl;
        return false;
    }
    return utils_math::FloatEqual(fltvalue, 0.f);
}

bool GetBoolFromBson(bson_t* bmeta, const char* key) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
        return GetBoolFromBsonIterator(&iter);
    }
    cout << "Failed in get boolean value of: " << key << endl;
    return false;
}

time_t GetDatetimeFromBsonIterator(bson_iter_t* iter) {
    const bson_value_t* vv = bson_iter_value(iter);
    if (vv->value_type == BSON_TYPE_DATE_TIME) {
        return CVT_TIMET(vv->value.v_datetime);
    }
    if (vv->value_type == BSON_TYPE_UTF8) {
        string tmp_time_str = vv->value.v_utf8.str;
        if (tmp_time_str.size() > 12) {
            return ConvertToTime(tmp_time_str, "%4d-%2d-%2d %2d:%2d:%2d", true);
        }
        return ConvertToTime(tmp_time_str, "%4d-%2d-%2d", false);
    }
    cout << "Failed in get Date Time value." << endl;
    return -1;
}

time_t GetDatetimeFromBson(bson_t* bmeta, const char* key) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, bmeta) && bson_iter_find(&iter, key)) {
        return GetDatetimeFromBsonIterator(&iter);
    }
    cout << "Failed in get Datetime value of: " << key << endl;
    return -1;
}

} /* namespace: db_mongoc */
} /* namespace: ccgl */
#endif /* USE_MONGODB */
