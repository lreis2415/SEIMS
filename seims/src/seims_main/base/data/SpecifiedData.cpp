#include "SpecifiedData.h"

void Read1DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &num, float *&data) {
    char *databuf = NULL;
    int datalength;
    MongoGridFS().getStreamData(remoteFilename, databuf, datalength, spatialData);
    float *floatValues = (float *) databuf;
    num = datalength / 4;
    data = (float *) databuf;
}

void Read2DArrayFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &rows, int &cols, float **&data) {
    char *databuf = NULL;
    int datalength;
    MongoGridFS().getStreamData(remoteFilename, databuf, datalength, spatialData);
    float *floatValues = (float *) databuf;

    int nRows = (int) floatValues[0];
    int nCols = -1;
    rows = nRows;
    data = new float *[rows];
    //cout<<n<<endl;
    int index = 1;
    for (int i = 0; i < rows; i++) {
        int col = int(floatValues[index]); // real column
        if (nCols < 0) {
            nCols = col;
        } else if (nCols != col) {
            nCols = 1;
        }
        int nSub = col + 1;
        data[i] = new float[nSub];
        data[i][0] = col;
        //cout<<"index: "<<index<<",";
        for (int j = 1; j < nSub; j++) {
            data[i][j] = floatValues[index + j];
            //cout<<data[i][j]<<",";
        }
        //cout<<endl;
        index += nSub;
    }
    cols = nCols;
}

void ReadIUHFromMongoDB(mongoc_gridfs_t *spatialData, string &remoteFilename, int &n, float **&data) {
    char *databuf = NULL;
    int datalength;
    MongoGridFS().getStreamData(remoteFilename, databuf, datalength, spatialData);
    float *floatValues = (float *) databuf;

    n = (int) floatValues[0];
    data = new float *[n];

    int index = 1;
    for (int i = 0; i < n; i++) {
        int nSub = (int) (floatValues[index + 1] - floatValues[index] + 3);
        data[i] = new float[nSub];

        data[i][0] = floatValues[index];
        data[i][1] = floatValues[index + 1];
        for (int j = 2; j < nSub; j++) {
            data[i][j] = floatValues[index + j];
        }
        index = index + nSub;
    }
    Release1DArray(floatValues);
    databuf = NULL;
}

void ReadLongTermMultiReachInfo(mongoc_client_t *conn, string &dbName, int &nr, int &nc, float **&data) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    bson_t *child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2);
    BSON_APPEND_INT32(child2, REACH_SUBBASIN, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection;
    bson_error_t *err = NULL;

    collection = mongoc_client_get_collection(conn, dbName.c_str(), DB_TAB_REACH);
    const bson_t *qCount = bson_new();
    int nReaches = (int) mongoc_collection_count(collection, MONGOC_QUERY_NONE, qCount, 0, 0, NULL, err);
    if (err != NULL || nReaches < 0) {
        throw ModelException("MongoUtil", "ReadLongTermMutltiReachInfo",
                             "Failed to get document number of collection: " + string(DB_TAB_REACH) + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    int nAttr = 10;
    float **tmpData = new float *[nReaches];
    for (int i = 0; i < nReaches; i++) {
        tmpData[i] = new float[nAttr];
        for (int j = 0; j < nAttr; j++) {
            tmpData[i][j] = 0.f;
        }
    }
    bson_iter_t itertor;
    int i = 0;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SUBBASIN)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][0]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNSTREAM)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][1]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_UPDOWN_ORDER)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][2]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_WIDTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][3]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_LENGTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][4]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DEPTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][5]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_V0)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][6]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_AREA)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][7]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_MANNING)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][8]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SLOPE)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][9]);
        }
        i++;
    }

    data = new float *[nAttr];
    for (i = 0; i < nAttr; i++) {
        data[i] = new float[nReaches + 1];
    }

    for (i = 0; i < nAttr; i++) {
        for (int j = 0; j < nReaches; j++) {
            data[i][j + 1] = tmpData[j][i];// index of the reach is the ID in the reach table (1 to nReaches)
        }
    }

    nr = nAttr;
    nc = nReaches + 1;

    for (i = 0; i < nReaches; i++) {
        delete[] tmpData[i];
    }
    delete[] tmpData;
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}

void ReadLongTermReachInfo(mongoc_client_t *conn, string &dbName, int subbasinID, int &nr, int &nc, float **&data) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, REACH_SUBBASIN, subbasinID); /// find the subbasin by given subbasinID
    BSON_APPEND_INT32(child1, REACH_GROUPDIVIDED, 1);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection;
    bson_error_t *err = NULL;

    collection = mongoc_client_get_collection(conn, dbName.c_str(), DB_TAB_REACH);
    if (err != NULL) {
        throw ModelException("MongoUtil", "ReadLongTermMutltiReachInfo",
                             "Failed to get collection: " + string(DB_TAB_REACH) + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    int nReaches = 1;
    int nAttr = 10;
    float **tmpData = new float *[nReaches];
    for (int i = 0; i < nReaches; i++) {
        tmpData[i] = new float[nAttr];
        for (int j = 0; j < nAttr; j++) {
            tmpData[i][j] = 0.f;
        }
    }

    bson_iter_t itertor;
    int i = 0;
    if (mongoc_cursor_next(cursor, &bsonTable)) {
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SUBBASIN)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][0]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNSTREAM)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][1]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_UPDOWN_ORDER)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][2]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_WIDTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][3]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_LENGTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][4]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DEPTH)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][5]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_V0)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][6]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_AREA)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][7]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_MANNING)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][8]);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SLOPE)) {
            GetNumericFromBsonIterator(&itertor, tmpData[i][9]);
        }
    } else {
        throw ModelException("MongoUtil", "ReadLongTermReachInfo", "Failed to query REACH information of SUBBASIN.\n");
    }

    data = new float *[nAttr];
    for (i = 0; i < nAttr; i++) {
        data[i] = new float[nReaches + 1];
    }

    for (i = 0; i < nAttr; i++) {
        for (int j = 0; j < nReaches; j++) {
            data[i][j + 1] = tmpData[j][i];
        }
    }

    nr = nAttr;
    nc = nReaches + 1;

    for (i = 0; i < nReaches; i++) {
        delete[] tmpData[i];
    }
    delete[] tmpData;
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}

void ReadMutltiReachInfoFromMongoDB(LayeringMethod layeringMethod, mongoc_client_t *conn, string &dbName, int &nr,
                                    int &nc, float **&data) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    bson_t *child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2);
    BSON_APPEND_INT32(child2, REACH_SUBBASIN, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection;
    bson_error_t *err = NULL;

    collection = mongoc_client_get_collection(conn, dbName.c_str(), DB_TAB_REACH);
//    int nReaches = (int) mongoc_collection_count(collection, MONGOC_QUERY_NONE, b, 0, 0, NULL, err);
    if (err != NULL) {
        throw ModelException("MongoUtil", "ReadMutltiReachInfoFromMongoDB",
                             "Failed to get document number of collection: " + string(DB_TAB_REACH) + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
    vector <vector<float>> vecReaches;
    float id = 0.f, upDownOrder = 0.f, downUpOrder = 0.f, downStreamID = 0.f, manning = 0.f, v0 = 0.f;

    bson_iter_t itertor;

    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SUBBASIN)) {
            GetNumericFromBsonIterator(&itertor, id);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNUP_ORDER)) {
            GetNumericFromBsonIterator(&itertor, downUpOrder);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_UPDOWN_ORDER)) {
            GetNumericFromBsonIterator(&itertor, upDownOrder);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNSTREAM)) {
            GetNumericFromBsonIterator(&itertor, downStreamID);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_MANNING)) {
            GetNumericFromBsonIterator(&itertor, manning);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_V0)) {
            GetNumericFromBsonIterator(&itertor, v0);
        }
        vector<float> vec;
        vec.resize(5);
        vec[0] = id;
        if (layeringMethod == UP_DOWN) {
            vec[1] = upDownOrder;//order
        } else {
            vec[1] = downUpOrder;
        }
        vec[2] = downStreamID;//downstream id
        vec[3] = manning;//manning's n
        vec[4] = v0; // v0

        vecReaches.push_back(vec);
    }
    int numRec = vecReaches.size();
    int nAttr = 5;
    data = new float *[nAttr];
    for (int i = 0; i < nAttr; i++) {
        data[i] = new float[numRec];
    }
    for (int i = 0; i < numRec; ++i) {
        data[0][i] = vecReaches[i][0];//subbasin id
        data[1][i] = vecReaches[i][1];//order
        data[2][i] = vecReaches[i][2];//downstream id
        data[3][i] = vecReaches[i][3];//manning's n
        data[4][i] = vecReaches[i][4];//v0
    }
    nr = nAttr;
    nc = numRec;
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}

void ReadReachInfoFromMongoDB(LayeringMethod layeringMethod, mongoc_client_t *conn, string &dbName, int subbasinID,
                              int &nr, int &nc, float **&data) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, REACH_SUBBASIN, subbasinID);
    BSON_APPEND_INT32(child1, REACH_GROUPDIVIDED, 1);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection;
    bson_error_t *err = NULL;

    collection = mongoc_client_get_collection(conn, dbName.c_str(), DB_TAB_REACH);
    if (err != NULL) {
        throw ModelException("MongoUtil", "ReadReachInfoFromMongoDB",
                             "Failed to get collection: " + string(DB_TAB_REACH) + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    int nAttr = 5;
    int nReaches = 1;
    data = new float *[nAttr];
    for (int i = 0; i < nAttr; i++) {
        data[i] = new float[nReaches];
    }

    float id = 0.f, upDownOrder = 0.f, downUpOrder = 0.f, downStreamID = 0.f, manning = 0.f, v0 = 0.f;
    bson_iter_t itertor;

    if (mongoc_cursor_next(cursor, &bsonTable)) {
        if (bson_iter_init_find(&itertor, bsonTable, REACH_SUBBASIN)) {
            GetNumericFromBsonIterator(&itertor, id);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNUP_ORDER)) {
            GetNumericFromBsonIterator(&itertor, downUpOrder);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_UPDOWN_ORDER)) {
            GetNumericFromBsonIterator(&itertor, upDownOrder);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_DOWNSTREAM)) {
            GetNumericFromBsonIterator(&itertor, downStreamID);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_MANNING)) {
            GetNumericFromBsonIterator(&itertor, manning);
        }
        if (bson_iter_init_find(&itertor, bsonTable, REACH_V0)) {
            GetNumericFromBsonIterator(&itertor, v0);
        }
    }
    for (int i = 0; i < nReaches; ++i) {
        data[0][i] = id;
        if (layeringMethod == UP_DOWN) {
            data[1][i] = upDownOrder;//order
        } else {
            data[1][i] = downUpOrder;
        }//downstream id
        data[2][i] = downStreamID;
        data[3][i] = manning;//manning's n
        data[4][i] = v0;
    }

    nr = nAttr;
    nc = nReaches;
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}
