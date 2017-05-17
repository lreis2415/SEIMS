#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include <iostream>
#include "MongoUtil.h"

using namespace std;
/*!
 * Define Raster related constant strings used for raster headers
 */
#define HEADER_RS_NODATA        "NODATA_VALUE"
#define HEADER_RS_XLL           "XLLCENTER"  /// or XLLCORNER
#define HEADER_RS_YLL           "YLLCENTER"  /// or YLLCORNER
#define HEADER_RS_NROWS         "NROWS"
#define HEADER_RS_NCOLS         "NCOLS"
#define HEADER_RS_CELLSIZE      "CELLSIZE"
#define HEADER_RS_LAYERS        "LAYERS"
#define HEADER_RS_SRS           "SRS"
const char *RASTER_HEADERS[8] = {HEADER_RS_NCOLS, HEADER_RS_NROWS, HEADER_RS_XLL, HEADER_RS_YLL, HEADER_RS_CELLSIZE,
                                 HEADER_RS_NODATA, HEADER_RS_LAYERS, HEADER_RS_SRS};


struct OrgOutItem
{
public:
    OrgOutItem() {};
    ~OrgOutItem() {};
public:
    string modCls;
    string outputID;
    string descprition;
    string outFileName;
    string aggType;
    string unit;
    string subBsn;
    string dataType;
    string intervalUnit;
    string sTimeStr;
    string eTimeStr;
    int interval;
    int use;
};

int main() {
    cout << "*** MongoUtil Demo ***" << endl;
    /*!
     * Get a MongoDB instance according to IP and port.
     * Exception will be throw if:
     *     an invalid IP.
     *     the MongoDB is unreachable.
     */
    cout << "Get a MongoDB instance" << endl;
    string ip = "127.0.0.1";
    int port = 27017;
    MongoClient *client = MongoClient::Init(ip.c_str(), port);
    if (NULL == client) { // return if connection to MongoDB failed
        return -1;
    }
    mongoc_client_t *conn = client->getConn();
    /*!
     * Get database names, a string vector is returned.
     */
    cout << "Get database names" << endl;
    vector<string> dbnames;
    client->getDatabaseNames(dbnames);
    for (vector<string>::iterator it = dbnames.begin(); it != dbnames.end(); it++) {
        cout << *it << endl;
    }
    cout << endl;

    /// Get Database
    string modelname = "model_dianbu2_30m_demo";
    mongoc_database_t *db = client->getDatabase(modelname);
    /// Get Collection
    string collname = "FILE_IN";
    mongoc_collection_t *coll = client->getCollection(modelname, collname);
    /// Fetch all the record in collection FILE_IN
    /*******Begin*********/
    bson_t* b = bson_new();

    unique_ptr<MongoCollection> collection(new MongoCollection(client->getCollection(modelname, collname)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    bson_iter_t itertor;
    const bson_t* bsonTable;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        if (bson_iter_init_find(&itertor, bsonTable, "TAG")) {
            cout << GetStringFromBsonIterator(&itertor) << ": ";
        }
        if (bson_iter_init_find(&itertor, bsonTable, "VALUE")) {
            cout << GetStringFromBsonIterator(&itertor) << endl;
        }
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);

    b = BCON_NEW("$query", "{", "NAME", "{", "$in", "[", BCON_UTF8("OUTLET_ID"),
        BCON_UTF8("SUBBASINID_NUM"), "]", "}", "}");

    //mongoc_collection_t* c2 = client->getCollection(modelname, "PARAMETERS");
    //cursor = mongoc_collection_find(c2, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
    //cursor = mongoc_collection_find_with_opts(c2, b, NULL, NULL);
    unique_ptr<MongoCollection> collection2(new MongoCollection(client->getCollection(modelname, "PARAMETERS")));
    cursor = collection2->ExecuteQuery(b);

    bson_error_t *err = NULL;
    if (mongoc_cursor_error(cursor, err)) {
        cout << "ERROR: Nothing found for subbasin number and outlet ID." << endl;
    }
    bson_iter_t iter;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        string nameTmp = "";
        int numTmp = -1;
        if (bson_iter_init_find(&iter, bsonTable, "NAME")) {
            nameTmp = GetStringFromBsonIterator(&iter);
            cout << "NAME: " << nameTmp;
        }
        if (bson_iter_init_find(&iter, bsonTable, "VALUE")) {
            GetNumericFromBsonIterator(&iter, numTmp);
            cout << ", VALUE: " << numTmp << endl;
        }
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);

    /*******End*********/

    /// Get collection names of a database
    cout << "Get collection names" << endl;
    //vector<string> colnames = client.getCollectionNames(string("model_dianbu2_30m_longterm"));
    /// another way
    vector <string> colnames;
    MongoDatabase(conn, string("model_dianbu2_30m_longterm")).getCollectionNames(colnames);
    for (vector<string>::iterator it = colnames.begin(); it != colnames.end(); it++) {
        cout << *it << endl;
    }
    cout << endl;
    /// Get GridFS
    mongoc_gridfs_t *gfs = client->getGridFS(string("model_dianbu2_30m_longterm"), string("SPATIAL"));
    /// Get GridFS file names
    cout << "Get GridFS file names" << endl;
    //vector<string> gfsfilenames = client.getGridFSFileNames(string("model_dianbu2_30m_longterm"), string("SPATIAL"));
    /// another way
    vector <string> gfsfilenames;
    MongoGridFS().getFileNames(gfsfilenames, gfs);
    /// or, however, in this way, gfs will be destroyed (released) afterwards.
    //vector<string> gfsfilenames = MongoGridFS(gfs).getFileNames();
    cout << "Totally " << gfsfilenames.size() << " files existed!" << endl;
    /// Get a given GridFS file data and metadata
    MongoGridFS mgfs = MongoGridFS(gfs);
    size_t length;
    char *databuf;
    mgfs.getStreamData(string("0_AWC"), databuf, length);
    bson_t *metadata = mgfs.getFileMetadata(string("0_AWC"));

    for (int i = 0; i < 7; i++) {
        double tmp;
        GetNumericFromBson(metadata, RASTER_HEADERS[i], tmp);
        cout << RASTER_HEADERS[i] << ": " << tmp << endl;
    }
    string srs = GetStringFromBson(metadata, HEADER_RS_SRS);
    cout << "srs: " << srs << endl;
    /// Write a GridFS file to MongoDB
    mgfs.writeStreamData(string("AWC"), databuf, length, metadata);
    Release1DArray(databuf);
    if (NULL != client) {
        delete client;
    }
    return 0;
}
