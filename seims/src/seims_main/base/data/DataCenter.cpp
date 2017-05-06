#include "DataCenter.h"

/******* DataCenter ********/
DataCenter::DataCenter(const string modelPath, const string modulePath,
                       const LayeringMethod layeringMethod /* = UP_DOWN */,
                       const int subBasinID /* = 0 */, const int scenarioID /* = -1 */,
                       const int numThread /* = 1 */):
        m_modelPath(modelPath), m_modulePath(modulePath),
        m_lyrMethod(layeringMethod), m_subbasinID(subBasinID), m_scenarioID(scenarioID),
        m_threadNum(numThread), m_useScenario(false){
    /// Get model name
    size_t nameIdx = modelPath.rfind(SEP);
    m_modelName = modelPath.substr(nameIdx + 1);
}

DataCenter::~DataCenter() {

}

vector<string>& DataCenter::getFileInStringVector(void) {
    if (m_fileIn1DStrs.empty()) {
        if (!LoadPlainTextFile(m_fileInFile, m_fileIn1DStrs)) {
            throw ModelException("DataCenter", "getFileInStringVector", "");
        }
    }
    return m_fileIn1DStrs;
}

/******* DataCenterMongoDB ********/
DataCenterMongoDB::DataCenterMongoDB(const char *host, const uint16_t port, const string modelPath,
                                     const string modulePath, const LayeringMethod layeringMethod /* = UP_DOWN */,
                                     const int subBasinID /* = 0 */, const int scenarioID /* = -1 */,
                                     const int numThread /* = 1 */):
                                     m_mongodbIP(host), m_mongodbPort(port), m_mongoClient(NULL), m_mainDatabase(NULL),
                                     m_climDatabase(NULL), m_scenDatabase(NULL), m_climDBName(""), m_scenDBName(""),
        DataCenter(modelPath, modulePath, layeringMethod, subBasinID, scenarioID, numThread) {
    /// Connect to MongoDB database, and make sure the required data is available.
    m_mongoClient = MongoClient::Init(m_mongodbIP, m_mongodbPort);
    if (NULL == m_mongoClient) {
        throw ModelException("DataCenterMongoDB", "Constructor", "Failed to connect to MongoDB!");
    }
    if (!CheckModelPreparedData()) {
        throw ModelException("DataCenterMongoDB", "CheckModelPreparedData", "Model data has been set up!");
    }
}

DataCenterMongoDB::~DataCenterMongoDB() {

}

bool DataCenterMongoDB::CheckModelPreparedData(void) {
    /// 1. Check and get the main model database
    vector<string> existedDBNames;
    m_mongoClient->getDatabaseNames(existedDBNames);
    if (!ValueInVector(m_modelName, existedDBNames)) {
        cout << "ERROR: The main model is not existed: " << m_modelName << endl;
        return false;
    }
    m_mainDatabase = new MongoDatabase(m_mongoClient->getDatabase(m_modelName));
    /// 2. Check the existence of FILE_IN, FILE_OUT, PARAMETERS, REACHES, SITELIST, SPATIAL, etc
    vector<string> existedMainDBTabs;
    m_mainDatabase->getCollectionNames(existedMainDBTabs);
    for (int i = 0; i < MAIN_DB_TABS_REQ_NUM; ++i) {
        if (!ValueInVector(MAIN_DB_TABS_REQ[i], existedMainDBTabs)) {
            cout << "ERROR: Table " << MAIN_DB_TABS_REQ[i] << " must be existed in " << m_modelName << endl;
            return false;
        }
    }
    /// 3. Check if Scenario will be applied, Get scenario database if necessary
    if (ValueInVector(string(DB_TAB_SCENARIO), existedMainDBTabs) && m_scenarioID >= 0) {
        m_useScenario = true;
        bson_t *query;
        query = bson_new();
        m_scenDBName = QueryDatabaseName(query, DB_TAB_SCENARIO);
        assert(m_scenDBName != "");
        m_scenDatabase = new MongoDatabase(m_mongoClient->getDatabase(m_scenDBName));
    }
    /// 4. Get climate database name from m_mainDatabase->SITELIST 
    getFileInStringVector();
    bson_t* query;
    query = bson_new(); 
    BSON_APPEND_INT32(query, Tag_SubbasinId, m_subbasinID); // subbasin id
    BSON_APPEND_UTF8(query, Tag_Mode, m_modelMode.c_str()); // model mode

    m_climDBName = QueryDatabaseName(query, DB_TAB_SITELIST);
    assert(m_climDBName != "");
    m_climDatabase = new MongoDatabase(m_mongoClient->getDatabase(m_climDBName));

    return true;
}

string DataCenterMongoDB::QueryDatabaseName(bson_t* query, const char* tabname) {
    unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, tabname)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);
    const bson_t* doc;
    string dbname = "";
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, "DB")) {
            dbname = GetStringFromBsonIterator(&iter);
            break;
        }
        else {
            cout << "ERROR: The DB field does not exist in " << string(tabname) << endl;
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return dbname;
}

vector<string>& DataCenterMongoDB::getFileInStringVector(void) {
    if (m_fileIn1DStrs.empty()) {
        bson_t* b = bson_new();
        bson_t* child1 = bson_new();
        BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
        bson_append_document_end(b, child1);
        bson_destroy(child1);

        mongoc_cursor_t* cursor;
        const bson_t* bsonTable;
        unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, DB_TAB_FILE_IN)));
        cursor = collection->ExecuteQuery(b);

        bson_iter_t itertor;
        while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
            vector<string> tokens(2);
            if (bson_iter_init_find(&itertor, bsonTable, Tag_ConfTag)) {
                tokens[0] = GetStringFromBsonIterator(&itertor);
            }
            if (bson_iter_init_find(&itertor, bsonTable, Tag_ConfValue)) {
                tokens[1] = GetStringFromBsonIterator(&itertor);
            }
            if (StringMatch(tokens[0], Tag_Mode)) {
                m_modelMode = tokens[1];
            }
            int sz = m_fileIn1DStrs.size(); // get the current number of rows
            m_fileIn1DStrs.resize(sz + 1);  // resize with one more row
            m_fileIn1DStrs[sz] = tokens[0] + "|" + tokens[1]; // keep the interface consistent
        }
        bson_destroy(b);
        mongoc_cursor_destroy(cursor);
    }
    return m_fileIn1DStrs;
}
