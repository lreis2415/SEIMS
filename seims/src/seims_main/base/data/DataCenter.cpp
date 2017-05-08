#include "DataCenter.h"

/******* DataCenter ********/
DataCenter::DataCenter(const string modelPath, const string modulePath,
                       const LayeringMethod layeringMethod /* = UP_DOWN */,
                       const int subBasinID /* = 0 */, const int scenarioID /* = -1 */,
                       const int numThread /* = 1 */):
        m_modelPath(modelPath), m_modulePath(modulePath),
        m_lyrMethod(layeringMethod), m_subbasinID(subBasinID), m_scenarioID(scenarioID),
        m_threadNum(numThread), m_useScenario(false), m_outputScene(DB_TAB_OUT_SPATIAL),
        m_outputPath(""),
        m_input(NULL), m_output(NULL), m_scenario(NULL) {
    /// Get model name
    size_t nameIdx = modelPath.rfind(SEP);
    m_modelName = modelPath.substr(nameIdx + 1);
    if (m_scenarioID >= 0) { // -1 means no BMPs scenario will be simulated
        m_outputScene += ValueToString(m_scenarioID);
    }
    checkConfigurationFiles();
    createOutputFolder();
}

DataCenter::~DataCenter() {
    StatusMessage("Release DataCenter...");
    if (m_scenario != NULL) {
        StatusMessage("---release bmps scenario data ...");
        delete m_scenario;
        m_scenario = NULL;
    }
    if (m_reaches != NULL) {
        StatusMessage("---release reaches data ...");
        delete m_reaches;
        m_reaches = NULL;
    }
    if (m_subbasins != NULL) {
        StatusMessage("---release subbasins data ...");
        delete m_subbasins;
        m_subbasins = NULL;
    }
}

bool DataCenter::checkConfigurationFiles(void) {
    string cfgNames[] = { File_Config, File_Input, File_Output };
    for (int i = 0; i < cfgNames->size(); ++i) {
        string checkFilePath = m_modelPath + SEP + cfgNames[i]; 
        if (!FileExists(checkFilePath)) {
            throw ModelException("DataCenter", "checkConfigurationFiles", checkFilePath +
                " does not exist or has not the read permission!");
            return false;
        }
    }
    return true;
}

bool DataCenter::createOutputFolder(void) {
    m_outputPath = m_modelPath + SEP + m_outputScene;
    return CleanDirectory(m_outputPath);
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
                                     m_spatialGridFS(NULL),
                                     m_climDBName(""), m_scenDBName(""),
        DataCenter(modelPath, modulePath, layeringMethod, subBasinID, scenarioID, numThread) {
    /// Connect to MongoDB database, and make sure the required data is available.
    m_mongoClient = MongoClient::Init(m_mongodbIP, m_mongodbPort);
    if (NULL == m_mongoClient) {
        throw ModelException("DataCenterMongoDB", "Constructor", "Failed to connect to MongoDB!");
    }
    m_input = new SettingsInput(getFileInStringVector());
    if (!getSubbasinNumberAndOutletID()) {
        throw ModelException("DataCenterMongoDB", "checkModelPreparedData", "Parameters has not been set up!");
    }
    m_output = new SettingsOutput(m_subbasinID, m_nSubbasins, m_outletID, getFileOutVector());
    /// Check the existence of all required and optional data
    if (!checkModelPreparedData()) {
        throw ModelException("DataCenterMongoDB", "checkModelPreparedData", "Model data has not been set up!");
    }

}

DataCenterMongoDB::~DataCenterMongoDB() {

}

bool DataCenterMongoDB::checkModelPreparedData(void) {
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
        bson_t *query;
        query = bson_new();
        m_scenDBName = QueryDatabaseName(query, DB_TAB_SCENARIO);
        if (m_scenDBName != "") {
            m_useScenario = true;
            m_scenario = new Scenario(m_mongoClient, m_scenDBName, m_scenarioID);
        }
    }
    /// 4. Read climate site information from Climate database
    m_climStation = new InputStation(m_mongoClient, m_input->getDtHillslope(), m_input->getDtChannel());
    readClimateSiteList();

    /// Read Reaches data
    m_reaches = new clsReaches(m_mongoClient, m_modelName, DB_TAB_REACH);
    /// Read Mask raster data
    m_spatialGridFS = new MongoGridFS(m_mongoClient->getGridFS(m_modelName, DB_TAB_SPATIAL));
    ostringstream oss;
    oss << m_subbasinID << "_" << Tag_Mask;
    string maskFileName = GetUpper(oss.str());
    m_maskRaster = new clsRasterData<float>(m_spatialGridFS, maskFileName.c_str());
    m_rsMap.insert(make_pair(maskFileName, m_maskRaster));
    /// Read Subbasin raster data
    oss.str("");
    oss << m_subbasinID << "_" << VAR_SUBBSN;
    string subbasinFileName = GetUpper(oss.str());
    clsRasterData<float> *subbasinRaster = new clsRasterData<float>(m_spatialGridFS,
        subbasinFileName.c_str(), true, m_maskRaster);
    m_rsMap.insert(make_pair(subbasinFileName, subbasinRaster));
    /// Read Subbasin data
    m_subbasins = new clsSubbasins(m_spatialGridFS, m_rsMap, m_subbasinID);

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

        unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, DB_TAB_FILE_IN)));
        mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

        bson_iter_t itertor;
        const bson_t* bsonTable;
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

vector<OriginalOutputItem>& DataCenterMongoDB::getFileOutVector(void) {
    vector<OriginalOutputItem> originOutputs;

    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, DB_TAB_FILE_OUT)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    bson_iter_t itertor;
    const bson_t *bsonTable;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        OriginalOutputItem tmpOutputItem;
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputUSE)) {
            GetNumericFromBsonIterator(&itertor, tmpOutputItem.use);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_MODCLS)) {
            tmpOutputItem.modCls = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputID)) {
            tmpOutputItem.outputID = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputDESC)) {
            tmpOutputItem.descprition = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_FileName)) {
            tmpOutputItem.outFileName = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_AggType)) {
            tmpOutputItem.aggType = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputUNIT)) {
            tmpOutputItem.unit = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_OutputSubbsn)) {
            tmpOutputItem.subBsn = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_StartTime)) {
            tmpOutputItem.sTimeStr = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_EndTime)) {
            tmpOutputItem.eTimeStr = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_Interval)) {
            GetNumericFromBsonIterator(&itertor, tmpOutputItem.interval);
        }
        if (bson_iter_init_find(&itertor, bsonTable, Tag_IntervalUnit)) {
            tmpOutputItem.intervalUnit = GetStringFromBsonIterator(&itertor);
        }
        if (tmpOutputItem.use <= 0) {
            continue;
        }
        else {
            originOutputs.push_back(tmpOutputItem);
        }
    }
    return originOutputs;
}

bool DataCenterMongoDB::getSubbasinNumberAndOutletID(void) {
    bson_t *b = bson_new();
    bson_t *child = bson_new();
    bson_t *child2 = bson_new();
    bson_t *child3 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child);
    BSON_APPEND_DOCUMENT_BEGIN(child, PARAM_FLD_NAME, child2);
    BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
    BSON_APPEND_UTF8(child3, PARAM_FLD_NAME, VAR_OUTLETID);
    BSON_APPEND_UTF8(child3, PARAM_FLD_NAME, VAR_SUBBSNID_NUM);
    bson_append_array_end(child2, child3);
    bson_append_document_end(child, child2);
    bson_append_document_end(b, child);
    //printf("%s\n",bson_as_json(b,NULL));

    unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, DB_TAB_PARAMETERS)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    bson_iter_t iter;
    const bson_t *bsonTable;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        string nameTmp = "";
        int numTmp = -1;
        if (bson_iter_init_find(&iter, bsonTable, PARAM_FLD_NAME)) {
            nameTmp = GetStringFromBsonIterator(&iter);
        }
        if (bson_iter_init_find(&iter, bsonTable, PARAM_FLD_VALUE)) {
            GetNumericFromBsonIterator(&iter, numTmp);
        }
        if (!StringMatch(nameTmp, "") && numTmp != -1) {
            if (StringMatch(nameTmp, VAR_OUTLETID)) {
                GetNumericFromBsonIterator(&iter, m_outletID);
            }
            else if (StringMatch(nameTmp, VAR_SUBBSNID_NUM)) {
                GetNumericFromBsonIterator(&iter, m_nSubbasins);
            }
        }
        else {
            throw ModelException("DataCenterMongoDB", "getSubbasinNumberAndOutletID", 
                "No valid values found in MongoDB!");
            return false;
        }
    }
    bson_destroy(child);
    bson_destroy(child2);
    bson_destroy(child3);
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
    return true;
}

void DataCenterMongoDB::readClimateSiteList(void)
{
    bson_t *query;
    query = bson_new();
    // subbasin id
    BSON_APPEND_INT32(query, Tag_SubbasinId, m_subbasinID);
    // mode
    string modelMode = m_input->getModelMode();
    BSON_APPEND_UTF8(query, Tag_Mode, modelMode.c_str());

    unique_ptr<MongoCollection> collection(new MongoCollection(m_mongoClient->getCollection(m_modelName, DB_TAB_SITELIST)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);

    const bson_t *doc;
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_SITELIST_DB)) {
            m_climDBName = GetStringFromBsonIterator(&iter);
        }
        else {
            throw ModelException("DataCenterMongoDB", "Constructor", "The DB field does not exist in SiteList table.");
        }
        string siteList = "";
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_M)) {
            siteList = GetStringFromBsonIterator(&iter);
            for (int i = 0; i < METEO_VARS_NUM; ++i) {
                m_climStation->ReadSitesData(m_climDBName, siteList, METEO_VARS[i], 
                    m_input->getStartTime(), m_input->getEndTime(), m_input->isStormMode());
            }
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_P)) {
            siteList = GetStringFromBsonIterator(&iter);
            m_climStation->ReadSitesData(m_climDBName, siteList, DataType_Precipitation, 
                m_input->getStartTime(), m_input->getEndTime(), m_input->isStormMode());
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_PET)) {
            siteList = GetStringFromBsonIterator(&iter);
            m_climStation->ReadSitesData(m_climDBName, siteList, DataType_PotentialEvapotranspiration,
                m_input->getStartTime(), m_input->getEndTime(), m_input->isStormMode());
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
}
