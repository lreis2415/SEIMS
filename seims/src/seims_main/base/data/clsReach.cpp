#include "clsReach.h"

using namespace std;

const int REACH_PARAM_NUM = 43;
const char *REACH_PARAM_NAME[] = {REACH_SUBBASIN, REACH_NUMCELLS, REACH_GROUP,              // 0-2
                                  REACH_GROUPDIVIDED, REACH_KMETIS, REACH_PMETIS,           // 3-5
                                  REACH_DOWNSTREAM, REACH_UPDOWN_ORDER, REACH_DOWNUP_ORDER, // 6-8
                                  REACH_WIDTH, REACH_SIDESLP, REACH_LENGTH, REACH_DEPTH,    // 9-12
                                  REACH_V0, REACH_AREA, REACH_MANNING, REACH_SLOPE,         // 13-16
                                  REACH_KBANK, REACH_KBED, REACH_COVER,
                                  REACH_EROD,         // 17-20
                                  REACH_BC1, REACH_BC2, REACH_BC3, REACH_BC4,               // 21-24
                                  REACH_RS1, REACH_RS2, REACH_RS3, REACH_RS4, REACH_RS5,    // 25-29
                                  REACH_RK1, REACH_RK2, REACH_RK3, REACH_RK4,               // 30-33
                                  REACH_DISOX, REACH_BOD, REACH_ALGAE,                      // 34-36
                                  REACH_ORGN, REACH_NH4, REACH_NO2, REACH_NO3,              // 37-40
                                  REACH_ORGP, REACH_SOLP, REACH_GWNO3, REACH_GWSOLP};       // 41-44

clsReach::clsReach(const bson_t *&bsonTable) {
    bson_iter_t iterator;
    for (int i = 0; i < REACH_PARAM_NUM; i++) {
        float tmp_param;
        if (bson_iter_init_find(&iterator, bsonTable, REACH_PARAM_NAME[i])) {
            if (GetNumericFromBsonIterator(&iterator, tmp_param)) {
                if (REACH_PARAM_NAME[i] == REACH_BOD && tmp_param < 1.e-6f) tmp_param = 1.e-6f;
                m_paramMap.insert(make_pair(REACH_PARAM_NAME[i], tmp_param));
            }
        }
    }
}

float clsReach::Get(const string &key) {
    auto it = m_paramMap.find(key);
    if (it != m_paramMap.end()) {
        return it->second;
    } else { return NODATA_VALUE; }
}

void clsReach::Set(const string &key, float value) {
    auto it = m_paramMap.find(key);
    if (it != m_paramMap.end()) {
        m_paramMap.at(key) = value;
    } else { m_paramMap.insert(make_pair(key, NODATA_VALUE)); }
}

clsReaches::clsReaches(MongoClient *conn, string &dbName, string collectionName) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    bson_t *child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);  /// query all records
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2); /// and order by subbasin ID
    BSON_APPEND_INT32(child2, REACH_SUBBASIN, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    unique_ptr<MongoCollection> collection(new MongoCollection(conn->getCollection(dbName, collectionName)));
    this->m_reachNum = collection->QueryRecordsCount();
    if (this->m_reachNum < 0) {
        bson_destroy(b);
        throw ModelException("clsReaches", "ReadAllReachInfo",
                             "Failed to get document number of collection: " + collectionName + ".\n");
    }
    this->m_reachUpStream.resize(this->m_reachNum + 1);

    mongoc_cursor_t *cursor = collection->ExecuteQuery(b);
    const bson_t *bsonTable;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        clsReach *curReach = new clsReach(bsonTable);
        int subID = (int) curReach->Get(REACH_SUBBASIN);
        this->m_reachesMap.insert(make_pair(subID, curReach));
    }
    // In SEIMS, reach ID is the same as Index of array and vector.
    for (int i = 1; i <= m_reachNum; i++) {
        int downStreamId = (int) (m_reachesMap.at(i)->Get(REACH_DOWNSTREAM));
        if (downStreamId <= 0 || downStreamId > m_reachNum) continue;
        m_reachUpStream[downStreamId].push_back(i);
    }

    m_reachLayers.clear();

    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

clsReaches::~clsReaches() {
    StatusMessage("Release clsReach...");
    if (!m_reachesMap.empty()) {
        for (auto iter = m_reachesMap.begin(); iter != m_reachesMap.end();) {
            if (nullptr != iter->second) {
                delete iter->second;
                iter->second = nullptr;
            }
            m_reachesMap.erase(iter++);
        }
        m_reachesMap.clear();
    }
    if (!m_reachesPropMap.empty()) {
        for (auto iter = m_reachesPropMap.begin(); iter != m_reachesPropMap.end();) {
            if (nullptr != iter->second) {
                Release1DArray(iter->second);
            }
            m_reachesPropMap.erase(iter++);
        }
        m_reachesPropMap.clear();
    }
}

void clsReaches::GetReachesSingleProperty(string key, float **data) {
    auto iter = m_reachesPropMap.find(key);
    if (iter == m_reachesPropMap.end()) {
        float *values = nullptr;
        Initialize1DArray(m_reachNum + 1, values, 0.f);
        values[0] = m_reachNum;
        for (auto it = m_reachesMap.begin(); it != m_reachesMap.end(); it++) {
            values[it->first] = it->second->Get(key);
        }
        *data = values;
    } else {
        *data = iter->second;
    }
}

map<int, vector<int> > clsReaches::GetReachLayers(LayeringMethod mtd /* = UP_DOWN */) {
    if (!m_reachLayers.empty()) { return m_reachLayers; }
    for (int i = 1; i <= m_reachNum; i++) {
        int order = (int) (m_reachesMap.at(i)->Get(REACH_UPDOWN_ORDER));
        if (mtd == DOWN_UP) order = (int) (m_reachesMap.at(i)->Get(REACH_DOWNUP_ORDER));
        if (m_reachLayers.find(order) == m_reachLayers.end()) {
            vector<int> tmp;
            tmp.push_back(i);
            m_reachLayers.insert(make_pair(order, tmp));
        } else { m_reachLayers[order].push_back(i); }
    }
    return m_reachLayers;
};

void clsReaches::Update(const map<string, ParamInfo *> &caliparams_map) {
    for (int i = 0; i < REACH_PARAM_NUM; i++) {
        auto it = caliparams_map.find(REACH_PARAM_NAME[i]);
        if (it != caliparams_map.end()) {
            ParamInfo *tmpParam = it->second;
            if ((StringMatch(tmpParam->Change, PARAM_CHANGE_RC) && FloatEqual(tmpParam->Impact, 1.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_AC) && FloatEqual(tmpParam->Impact, 0.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_VC) && FloatEqual(tmpParam->Impact, NODATA_VALUE))) {
                continue;
            }
            for (auto it2 = m_reachesMap.begin(); it2 != m_reachesMap.end(); it2++) {
                float pre_value = it2->second->Get(REACH_PARAM_NAME[i]);
                if (FloatEqual(pre_value, NODATA_VALUE)) continue;
                float new_value = it->second->GetAdjustedValue(pre_value);
                it2->second->Set(REACH_PARAM_NAME[i], new_value);
            }
        }
    }
}