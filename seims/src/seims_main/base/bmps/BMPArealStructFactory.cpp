#include "BMPArealStructFactory.h"

using namespace MainBMP;

BMPArealStruct::BMPArealStruct(const bson_t *&bsonTable, bson_iter_t &iter):
m_name(""), m_desc(""), m_refer(""), m_id(-1)
{
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SUB)) {
        GetNumericFromBsonIterator(&iter, m_id);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_NAME)) {
        m_name = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_DESC)) {
        m_desc = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_REF)) {
        m_refer = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_LANDUSE)) {
        string landuse_str = GetStringFromBsonIterator(&iter);
        m_landuse = SplitStringForInt(landuse_str, '-');
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_ARSTRUCT_FLD_PARAMS)) {
        string params_str = GetStringFromBsonIterator(&iter);
        vector<string> params_strs = SplitString(params_str, '-');
        for (auto it = params_strs.begin(); it != params_strs.end(); it++) {
            vector<string> tmp_param_items = SplitString(*it, ':');
            assert(tmp_param_items.size() == 4);
            ParamInfo* p = new ParamInfo();
            p->Name = tmp_param_items[0];
            p->Description = tmp_param_items[1];
            p->Change = tmp_param_items[2];  /// can be "RC", "AC", "NC", "VC", and "".
            p->Impact = (float)atof(tmp_param_items[3].c_str());

            if (!m_parameters.insert(make_pair(GetUpper(p->Name), p)).second) {
                cout << "WARNING: Load parameter during constructing BMPArealStructFactory, BMPID: " 
                    << m_id << ", param_name: " << tmp_param_items[0] << endl;
            }
        }
    }
}

BMPArealStruct::~BMPArealStruct() {
    StatusMessage("---release map of parameters in BMPArealStruct ...");
    for (auto it = m_parameters.begin(); it != m_parameters.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_parameters.erase(it++);
    }
    m_parameters.clear();
}

BMPArealStructFactory::BMPArealStructFactory(int scenarioId, int bmpId, int subScenario,
                                             int bmpType, int bmpPriority, vector<string> &distribution,
                                             const string& collection, const string& location):
    BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location), 
    m_mgtFieldsRs(nullptr)
{
    if (m_distribution.size() >= 2 && StringMatch(m_distribution[0], FLD_SCENARIO_DIST_RASTER)) {
        m_mgtFieldsName = m_distribution[1];
    }
    else {
        throw ModelException("BMPArealStructFactory", "Initialization",
            "The distribution field must follow the format: "
            "RASTER|CoreRasterName.\n");
    }
    m_unitIDs = SplitStringForInt(location, '-');
}

BMPArealStructFactory::~BMPArealStructFactory()
{
	// m_mgtFieldsRs will be released in DataCenter. No need to be released here.
    for (auto it = m_bmpStructMap.begin(); it != m_bmpStructMap.end(); ) {
        if (nullptr != it->second) {
            delete it->second;
            it->second = nullptr;
        }
        m_bmpStructMap.erase(it++);
    }
    m_bmpStructMap.clear();
}

void BMPArealStructFactory::loadBMP(MongoClient* conn, const string &bmpDBName)
{
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, BMP_FLD_SUB, m_subScenarioId);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    unique_ptr<MongoCollection> collection(new MongoCollection(conn->getCollection(bmpDBName, m_bmpCollection)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    bson_iter_t iter;
    const bson_t *bsonTable;

    /// Use count to counting sequence number, in case of discontinuous or repeat of SEQUENCE in database.
    while (mongoc_cursor_next(cursor, &bsonTable)) {
        if (!m_bmpStructMap.insert(make_pair(m_subScenarioId, new BMPArealStruct(bsonTable, iter))).second) {
            cout << "WARNING: Read Areal Structural BMP failed: subScenarioID: " << m_subScenarioId << endl;
        }
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

void BMPArealStructFactory::setRasterData(map<string, FloatRaster*> &sceneRsMap) {
    if (sceneRsMap.find(m_mgtFieldsName) != sceneRsMap.end()) {
        int n;
        sceneRsMap.at(m_mgtFieldsName)->getRasterData(&n, &m_mgtFieldsRs);
    }
    else{
        // raise Exception?
    }
}

void BMPArealStructFactory::Dump(ostream *fs)
{
	if (nullptr == fs) return;
	*fs << "Areal Structural BMP Management Factory: " << endl <<
		"    SubScenario ID: " << m_subScenarioId << endl;
}
