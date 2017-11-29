#include "BMPPlantMgtFactory.h"

using namespace MainBMP;

BMPPlantMgtFactory::BMPPlantMgtFactory(int scenarioId, int bmpId, int subScenario,
                                       int bmpType, int bmpPriority, vector<string> &distribution,
                                       const string& collection, const string& location) :
    BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location)
{
    if (m_distribution.size() >= 2 && StringMatch(m_distribution[0], FLD_SCENARIO_DIST_RASTER)) {
        m_mgtFieldsName = m_distribution[1];
    }
    else {
        throw ModelException("BMPPlantMgtFactory", "Initialization",
            "The distribution field must follow the format: "
            "RASTER|CoreRasterName.\n");
    }
    if (StringMatch(location, "ALL")) {
        m_location.clear();
    } else {
        m_location = SplitStringForInt(location, '-');
    }
}

BMPPlantMgtFactory::~BMPPlantMgtFactory() {
    Release1DArray(m_parameters);
    for (auto it = m_bmpPlantOps.begin(); it != m_bmpPlantOps.end();) {
        if (nullptr != it->second) {
            delete it->second;
            it->second = nullptr;
        }
        m_bmpPlantOps.erase(it++);
    }
    m_bmpPlantOps.clear();
}

void BMPPlantMgtFactory::loadBMP(MongoClient* conn, const string &bmpDBName) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new(), *child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, FLD_SCENARIO_SUB, m_subScenarioId);
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2);
    BSON_APPEND_INT32(child2, BMP_FLD_SEQUENCE, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    unique_ptr<MongoCollection> collection(new MongoCollection(conn->getCollection(bmpDBName, m_bmpCollection)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);

    const bson_t *bsonTable;
    bson_iter_t itertor;
    int paramNum = 10;
    m_parameters = new float[paramNum];
    int count = 1; /// Use count to counting sequence number, in case of discontinuous of SEQUENCE in database.
    while (mongoc_cursor_next(cursor, &bsonTable)) {
        //int seqNo;
        int mgtCode = -1, year = -9999, month = -9999, day = -9999;
        float husc = 0.f;
        bool usebaseHU = true;
        if (bson_iter_init_find(&itertor, bsonTable, BMP_FLD_NAME)) {
            m_name = GetStringFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_LUCC)) {
            GetNumericFromBsonIterator(&itertor, m_luccID);
        }
        if (m_luccID < 0) m_luccID = 1; // AGRL	Agricultural Land-Generic
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_MGTOP)) {
            GetNumericFromBsonIterator(&itertor, mgtCode);
        }
        //if (bson_iter_init_find(&itertor,bsonTable,BMP_FLD_SEQUENCE))
        //	GetNumericFromBsonIterator(&itertor, seqNo);
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_YEAR)) {
            GetNumericFromBsonIterator(&itertor, year);
        }
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_MONTH)) {
            GetNumericFromBsonIterator(&itertor, month);
        }
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_DAY)) {
            GetNumericFromBsonIterator(&itertor, day);
        }
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_BASEHU)) {
            usebaseHU = GetBoolFromBsonIterator(&itertor);
        }
        if (bson_iter_init_find(&itertor, bsonTable, BMP_PLTOP_FLD_HUSC)) {
            GetNumericFromBsonIterator(&itertor, husc);
        }
        for (int i = 0; i < paramNum; i++) {
            ostringstream oss;
            oss << BMP_PLTOP_FLD_MGT_PRE << (i + 1);
            if (bson_iter_init_find(&itertor, bsonTable, oss.str().c_str())) {
                GetNumericFromBsonIterator(&itertor, m_parameters[i]);
            }
        }
        int uniqueMgtCode = count * 1000 + mgtCode;
        m_bmpSequence.push_back(uniqueMgtCode);
        switch (mgtCode) {
            case BMP_PLTOP_Plant:
                m_bmpPlantOps[uniqueMgtCode] = new PlantOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                  m_parameters);
                break;
            case BMP_PLTOP_Irrigation:
                m_bmpPlantOps[uniqueMgtCode] = new IrrigationOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                       m_parameters);
                break;
            case BMP_PLTOP_Fertilizer:
                m_bmpPlantOps[uniqueMgtCode] = new FertilizerOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                       m_parameters);
                break;
            case BMP_PLTOP_Pesticide:
                m_bmpPlantOps[uniqueMgtCode] = new PesticideOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                      m_parameters);
                break;
            case BMP_PLTOP_HarvestKill:
                m_bmpPlantOps[uniqueMgtCode] = new HarvestKillOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                        m_parameters);
                break;
            case BMP_PLTOP_Tillage:
                m_bmpPlantOps[uniqueMgtCode] = new TillageOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                    m_parameters);
                break;
            case BMP_PLTOP_Harvest:
                m_bmpPlantOps[uniqueMgtCode] = new HarvestOnlyOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                        m_parameters);
                break;
            case BMP_PLTOP_Kill:
                m_bmpPlantOps[uniqueMgtCode] = new KillOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                 m_parameters);
                break;
            case BMP_PLTOP_Grazing:
                m_bmpPlantOps[uniqueMgtCode] = new GrazingOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                    m_parameters);
                break;
            case BMP_PLTOP_AutoIrrigation:
                m_bmpPlantOps[uniqueMgtCode] = new AutoIrrigationOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                           m_parameters);
                break;
            case BMP_PLTOP_AutoFertilizer:
                m_bmpPlantOps[uniqueMgtCode] = new AutoFertilizerOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                           m_parameters);
                break;
            case BMP_PLTOP_ReleaseImpound:
                m_bmpPlantOps[uniqueMgtCode] = new ReleaseImpoundOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                           m_parameters);
                break;
            case BMP_PLTOP_ContinuousFertilizer:
                m_bmpPlantOps[uniqueMgtCode] = new ContinuousFertilizerOperation(mgtCode, usebaseHU, husc, year, month,
                                                                                 day, m_parameters);
                break;
            case BMP_PLTOP_ContinuousPesticide:
                m_bmpPlantOps[uniqueMgtCode] = new ContinuousPesticideOperation(mgtCode, usebaseHU, husc, year, month,
                                                                                day, m_parameters);
                break;
            case BMP_PLTOP_Burning:
                m_bmpPlantOps[uniqueMgtCode] = new BurningOperation(mgtCode, usebaseHU, husc, year, month, day,
                                                                    m_parameters);
                break;
            default:break;
        }
        count++;
    }
    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

void BMPPlantMgtFactory::Dump(ostream *fs) {
    if (nullptr == fs) return;
    *fs << "Plant Management Factory: " << endl <<
        "    SubScenario ID: " << m_subScenarioId << " Name = " << m_name << endl;
    for (auto it = m_bmpSequence.begin(); it != m_bmpSequence.end(); it++) {
        auto findIdx = m_bmpPlantOps.find(*it);
        if (findIdx != m_bmpPlantOps.end()) {
            m_bmpPlantOps[*it]->dump(fs);
        }
    }
}

void BMPPlantMgtFactory::setRasterData(map<string, FloatRaster*> &sceneRsMap) {
    if (sceneRsMap.find(m_mgtFieldsName) != sceneRsMap.end()) {
        int n;
        sceneRsMap.at(m_mgtFieldsName)->getRasterData(&n, &m_mgtFieldsRs);
    }
    else{
        // raise Exception?
    }
}
