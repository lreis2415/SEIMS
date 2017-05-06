#include "SettingsInput.h"

using namespace std;

//SettingsInput::SettingsInput(string fileName, mongoc_client_t *conn, string dbName, int nSubbasin)
//    : m_conn(conn), m_dbName(dbName), m_subbasinID(nSubbasin) {
//    LoadSettingsFromFile(fileName, dbName);
//    if (!readDate()) {
//        throw ModelException("SettingsInput",
//                             "LoadSettingsFromFile",
//                             "The start time and end time in file.in is invalid or missed. The format would be YYYY/MM/DD/HH. Please check it.");
//    }
//    m_inputStation = new InputStation(m_conn, m_dtHs, m_dtCh);
//    ReadSiteList();
//}
//
//SettingsInput::SettingsInput(mongoc_client_t *conn, string dbName, int nSubbasin)
//    : m_conn(conn), m_dbName(dbName), m_subbasinID(nSubbasin) {
//    LoadSettingsInputFromMongoDB();
//    if (!readDate()) {
//        throw ModelException("SettingsInput",
//                             "LoadSettingsFromMongoDB",
//                             "The start time and end time in file.in is invalid or missed. The format would be YYYY/MM/DD/HH. Please check it.");
//    }
//    m_inputStation = new InputStation(m_conn, m_dtHs, m_dtCh);
//    ReadSiteList();
//}

SettingsInput::SettingsInput(unique_ptr<DataCenter>& dcenter) : m_isStormModel(false), Settings() {
    Settings::SetSettingTagStrings(dcenter->getFileInStringVector());
    if (StringMatch(Settings::GetValue(Tag_Mode), Tag_Mode_Storm)) {
        m_isStormModel = true;
    }
    if (!readDate()) {
        throw ModelException("SettingsInput",
                                "LoadSettingsFromFile",
                                "The start time and end time in file.in is invalid or missed. The format would be YYYY/MM/DD/HH. Please check it.");
    }
    m_inputStation = new InputStation(m_conn, m_dtHs, m_dtCh);
    ReadSiteList();
}

SettingsInput::~SettingsInput(void) {
    StatusMessage("Start to release SettingsInput ...");
    if (m_inputStation != NULL) delete m_inputStation;
    StatusMessage("End to release SettingsInput ...");
}

bool SettingsInput::readDate() {
    //read start and end time
    m_startDate = ConvertToTime2(GetValue(Tag_StartTime), "%d-%d-%d %d:%d:%d", true);
    m_endDate = ConvertToTime2(GetValue(Tag_EndTime), "%d-%d-%d %d:%d:%d", true);

    if (m_startDate == -1 || m_endDate == -1) return false;

    // make sure the start and end times are in the proper order
    if (m_endDate < m_startDate) {
        time_t tmp = m_startDate;
        m_startDate = m_endDate;
        m_endDate = tmp;
    }

    m_mode = GetUpper(GetValue(Tag_Mode));

    //read interval
    vector <string> dtList = SplitString(GetValue(Tag_Interval), ',');
    m_dtHs = atoi(dtList[0].c_str());
    m_dtCh = m_dtHs;
    if (dtList.size() > 1) {
        m_dtCh = atoi(dtList[1].c_str());
    }
    // convert the time interval to seconds to conform to time_t struct
    if (StringMatch(m_mode, Tag_Mode_Daily)) {
        m_dtHs = 86400; // 86400 secs is 1 day
        m_dtCh = 86400;
    }
    return true;
}

void SettingsInput::ReadSiteList() {
    bson_t *query;
    query = bson_new();
    // subbasin id
    BSON_APPEND_INT32(query, Tag_SubbasinId, m_subbasinID);
    // mode
    BSON_APPEND_UTF8(query, Tag_Mode, m_mode.c_str());

    //string siteListTable = DB_TAB_SITELIST;

    mongoc_cursor_t *cursor;
    mongoc_collection_t *collection;
    const bson_t *doc;
    collection = mongoc_client_get_collection(m_conn, m_dbName.c_str(), DB_TAB_SITELIST);
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_SITELIST_DB)) {
            m_dbHydro = GetStringFromBsonIterator(&iter);
        } else {
            throw ModelException("SettingsInput", "ReadSiteList", "The DB field does not exist in SiteList table.");
        }
        string siteList = "";
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_M)) {
            siteList = GetStringFromBsonIterator(&iter);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_MeanTemperature, m_startDate, m_endDate);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_MaximumTemperature, m_startDate, m_endDate);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_MinimumTemperature, m_startDate, m_endDate);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_WindSpeed, m_startDate, m_endDate);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_SolarRadiation, m_startDate, m_endDate);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_RelativeAirMoisture, m_startDate, m_endDate);
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_P)) {
            siteList = GetStringFromBsonIterator(&iter);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_Precipitation, m_startDate, m_endDate,
                                          m_isStormModel);
        }

        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, SITELIST_TABLE_PET)) {
            siteList = GetStringFromBsonIterator(&iter);
            m_inputStation->ReadSitesData(m_dbHydro, siteList, DataType_PotentialEvapotranspiration, m_startDate,
                                          m_endDate, m_isStormModel);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
}

//bool SettingsInput::LoadSettingsFromFile(string filename, string dbName) {
//    //first get the SettingStrings from base class LoadSettingsFromFile function
//    if (!Settings::LoadSettingsFromFile(filename)) {
//        throw ModelException("SettingsInput", "LoadSettingsFromFile", "The file.in is invalid. Please check it.");
//    }
//    return true;
//}

//bool SettingsInput::LoadSettingsInputFromMongoDB() {
//    bson_t *b = bson_new();
//    bson_t *child1 = bson_new();
//    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
//    bson_append_document_end(b, child1);
//    bson_destroy(child1);
//
//    mongoc_cursor_t *cursor;
//    const bson_t *bsonTable;
//    mongoc_collection_t *collection;
//
//    collection = mongoc_client_get_collection(m_conn, m_dbName.c_str(), DB_TAB_FILE_IN);
//    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
//
//    bson_iter_t itertor;
//    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
//        vector <string> tokens(2);
//        if (bson_iter_init_find(&itertor, bsonTable, Tag_ConfTag)) {
//            tokens[0] = GetStringFromBsonIterator(&itertor);
//        }
//        if (bson_iter_init_find(&itertor, bsonTable, Tag_ConfValue)) {
//            tokens[1] = GetStringFromBsonIterator(&itertor);
//        }
//        int sz = m_Settings.size();        // get the current number of rows
//        m_Settings.resize(sz + 1);        // resize with one more row
//        m_Settings[sz] = tokens;
//    }
//    bson_destroy(b);
//    mongoc_collection_destroy(collection);
//    mongoc_cursor_destroy(cursor);
//    return true;
//}

void SettingsInput::Dump(string fileName) {
    ofstream fs;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open()) {
        fs << "Start Date :" << ConvertToString2(&m_startDate) << endl;
        fs << "End Date :" << ConvertToString2(&m_endDate) << endl;
        fs << "Interval :" << m_dtHs << "\t" << m_dtCh << endl;
        fs.close();
    }
}
