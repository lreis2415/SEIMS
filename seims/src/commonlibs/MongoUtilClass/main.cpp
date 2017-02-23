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
    MongoClient client = MongoClient(ip.c_str(), port);
    mongoc_client_t *conn = client.getConn();

    /*!
     * Get database names, a string vector is returned.
     */
    cout << "Get database names" << endl;
    vector <string> dbnames = client.getDatabaseNames();
    for (vector<string>::iterator it = dbnames.begin(); it != dbnames.end(); it++) {
        cout << *it << endl;
    }
    cout << endl;

    /// Get Database
    mongoc_database_t *db = client.getDatabase(string("model_dianbu2_30m_longterm"));
    /// Get Collection
    mongoc_collection_t *coll = client.getCollection(string("model_dianbu2_30m_longterm"), string("FILE_IN"));
    /// Get collection names of a database
    cout << "Get collection names" << endl;
    //vector<string> colnames = client.getCollectionNames(string("model_dianbu2_30m_longterm"));
    /// another way
    vector <string> colnames = MongoDatabase(conn, string("model_dianbu2_30m_longterm")).getCollectionNames();
    for (vector<string>::iterator it = colnames.begin(); it != colnames.end(); it++) {
        cout << *it << endl;
    }
    cout << endl;
    /// Get GridFS
    mongoc_gridfs_t *gfs = client.getGridFS(string("model_dianbu2_30m_longterm"), string("SPATIAL"));
    /// Get GridFS file names
    cout << "Get GridFS file names" << endl;
    //vector<string> gfsfilenames = client.getGridFSFileNames(string("model_dianbu2_30m_longterm"), string("SPATIAL"));
    /// another way
    vector <string> gfsfilenames = MongoGridFS().getFileNames(gfs);
    /// or, however, in this way, gfs will be destroyed (released) afterwards.
    //vector<string> gfsfilenames = MongoGridFS(gfs).getFileNames();
    cout << "Totally " << gfsfilenames.size() << " files existed!" << endl;
    /// Get a given GridFS file data and metadata
    MongoGridFS mgfs = MongoGridFS(gfs);
    int length;
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
    return 0;
}
