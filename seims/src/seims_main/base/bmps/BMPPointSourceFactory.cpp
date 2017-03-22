#include "BMPPointSourceFactory.h"

using namespace MainBMP;

BMPPointSrcFactory::BMPPointSrcFactory(int scenarioId, int bmpId, int subScenario, int bmpType, int bmpPriority,
                                       string distribution, string collection, string location)
    : BMPFactory(scenarioId, bmpId, subScenario, bmpType, bmpPriority, distribution, collection, location) {
    m_pointSrcMgtTab = m_bmpCollection;
    m_pointSrcIDs = SplitStringForInt(location, ',');
    vector <string> dist = SplitString(distribution, '|');
    if (dist.size() == 3 && StringMatch(dist[0], FLD_SCENARIO_DIST_ARRAY)) {
        m_pointSrcDistTab = dist[1];
        m_pointSrc = atoi(dist[2].c_str());
    } else {
        throw ModelException("BMPPointSourceFactory", "Initialization",
                             "The distribution field must follow the format: ARRAY|CollectionName|PTSRC.\n");
    }
}

BMPPointSrcFactory::~BMPPointSrcFactory(void) {
    if (!m_pointSrcLocsMap.empty()) {
        for (map<int, PointSourceLocations *>::iterator it = m_pointSrcLocsMap.begin();
             it != m_pointSrcLocsMap.end();) {
            if (it->second != NULL) {
                delete it->second;
                it->second = NULL;
            }
            it = m_pointSrcLocsMap.erase(it);
        }
        m_pointSrcLocsMap.clear();
    }
    if (!m_pointSrcMgtMap.empty()) {
        for (map<int, PointSourceMgtParams *>::iterator it = m_pointSrcMgtMap.begin();
             it != m_pointSrcMgtMap.end();) {
            if (it->second != NULL) {
                delete it->second;
                it->second = NULL;
            }
            it = m_pointSrcMgtMap.erase(it);
        }
        m_pointSrcMgtMap.clear();
    }
}

void BMPPointSrcFactory::loadBMP(mongoc_client_t *conn, string &bmpDBName) {
    ReadPointSourceManagements(conn, bmpDBName);
    ReadPointSourceLocations(conn, bmpDBName);
}

void BMPPointSrcFactory::ReadPointSourceManagements(mongoc_client_t *conn, string &bmpDBName) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new(), *child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, BMP_FLD_SUB, m_subScenarioId);
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2);
    BSON_APPEND_INT32(child2, BMP_FLD_SEQUENCE, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection = NULL;
    bson_iter_t iter;

    collection = mongoc_client_get_collection(conn, bmpDBName.c_str(), m_pointSrcMgtTab.c_str());
    if (collection == NULL) {
        throw ModelException("BMPs Scenario", "Read Point Source Management",
                             "Failed to get collection: " + m_pointSrcMgtTab + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);
    /// Use count to counting sequence number, in case of discontinuous or repeat of SEQUENCE in database.
    int count = 1;
    while (mongoc_cursor_next(cursor, &bsonTable)) {
        m_pointSrcMgtSeqs.push_back(count);
        m_pointSrcMgtMap[count] = new PointSourceMgtParams(bsonTable, iter);
        count++;
    }
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}

void BMPPointSrcFactory::ReadPointSourceLocations(mongoc_client_t *conn, string &bmpDBName) {
    bson_t *b = bson_new();
    bson_t *child1 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);
    BSON_APPEND_INT32(child1, BMP_PTSRC_FLD_CODE, m_pointSrc);
    bson_append_document_end(b, child1);
    bson_destroy(child1);

    mongoc_cursor_t *cursor;
    const bson_t *bsonTable;
    mongoc_collection_t *collection = NULL;
    bson_iter_t iter;

    collection = mongoc_client_get_collection(conn, bmpDBName.c_str(), m_pointSrcDistTab.c_str());
    if (collection == NULL) {
        throw ModelException("BMPs Scenario", "Read Point Source Locations",
                             "Failed to get collection: " + m_pointSrcDistTab + ".\n");
    }
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    while (mongoc_cursor_next(cursor, &bsonTable)) {
        PointSourceLocations *curPtSrcLoc = new PointSourceLocations(bsonTable, iter);
        int curPtSrcID = curPtSrcLoc->GetPointSourceID();
        if (ValueInVector(curPtSrcID, m_pointSrcIDs)) {
            m_pointSrcLocsMap[curPtSrcID] = curPtSrcLoc;
        } else {
            RemoveValueInVector(curPtSrcID, m_pointSrcIDs);
        }
    }
    bson_destroy(b);
    mongoc_collection_destroy(collection);
    mongoc_cursor_destroy(cursor);
}

void BMPPointSrcFactory::Dump(ostream *fs) {
    if (fs == NULL) return;
    *fs << "Point Source Management Factory: " << endl <<
        "    SubScenario ID: " << m_subScenarioId << " PTSRC: " << m_pointSrc << endl;
    for (vector<int>::iterator it = m_pointSrcMgtSeqs.begin(); it != m_pointSrcMgtSeqs.end(); it++) {
        map<int, PointSourceMgtParams *>::iterator findIdx = m_pointSrcMgtMap.find(*it);
        if (findIdx != m_pointSrcMgtMap.end()) {
            m_pointSrcMgtMap[*it]->Dump(fs);
        }
    }
    for (vector<int>::iterator it = m_pointSrcIDs.begin(); it != m_pointSrcIDs.end(); it++) {
        map<int, PointSourceLocations *>::iterator findIdx = m_pointSrcLocsMap.find(*it);
        if (findIdx != m_pointSrcLocsMap.end()) {
            m_pointSrcLocsMap[*it]->Dump(fs);
        }
    }
}


/************************************************************************/
/*                  PointSourceMgtParams                                        */
/************************************************************************/

PointSourceMgtParams::PointSourceMgtParams(const bson_t *&bsonTable, bson_iter_t &iter)
    : m_startDate(0), m_endDate(0), m_waterVolume(0.f), m_sedimentConc(0.f), m_TNConc(0.f), m_NO3Conc(0.f),
      m_NH4Conc(0.f), m_OrgNConc(0.f), m_TPConc(0.f), m_SolPConc(0.f), m_OrgPConc(0.f), m_COD(0.f),
      m_name(""), m_seqence(-1) {
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_NAME)) {
        m_name = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SEQUENCE)) {
        GetNumericFromBsonIterator(&iter, m_seqence);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_Q)) {
        GetNumericFromBsonIterator(&iter, m_waterVolume);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_SED)) {
        GetNumericFromBsonIterator(&iter, m_sedimentConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_TN)) {
        GetNumericFromBsonIterator(&iter, m_TNConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_NO3)) {
        GetNumericFromBsonIterator(&iter, m_NO3Conc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_NH4)) {
        GetNumericFromBsonIterator(&iter, m_NH4Conc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_ORGN)) {
        GetNumericFromBsonIterator(&iter, m_OrgNConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_TP)) {
        GetNumericFromBsonIterator(&iter, m_TPConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_SOLP)) {
        GetNumericFromBsonIterator(&iter, m_SolPConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_ORGP)) {
        GetNumericFromBsonIterator(&iter, m_OrgPConc);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_COD)) {
        GetNumericFromBsonIterator(&iter, m_COD);
    }
    int sYear, sMonth, sDay, eYear, eMonth, eDay;
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SYEAR)) {
        GetNumericFromBsonIterator(&iter, sYear);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SMONTH)) {
        GetNumericFromBsonIterator(&iter, sMonth);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_SDAY)) {
        GetNumericFromBsonIterator(&iter, sDay);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_EYEAR)) {
        GetNumericFromBsonIterator(&iter, eYear);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_EMONTH)) {
        GetNumericFromBsonIterator(&iter, eMonth);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_EDAY)) {
        GetNumericFromBsonIterator(&iter, eDay);
    }
    if (sYear > 0 && sMonth > 0 && sDay > 0) {
        m_startDate = ConvertYMDToTime(sYear, sMonth, sDay);
    }
    if (eYear > 0 && eMonth > 0 && eDay > 0) {
        m_endDate = ConvertYMDToTime(eYear, eMonth, eDay);
    }
}

PointSourceMgtParams::~PointSourceMgtParams(void) {
}

void PointSourceMgtParams::Dump(ostream *fs) {
    if (fs == NULL) return;
    *fs << "    Point Source Managements: " << endl;
    if (m_startDate != 0) {
        *fs << "      Start Date: " << ConvertToString(&m_startDate) << endl;
    }
    if (m_endDate != 0) {
        *fs << "      End Date: " << ConvertToString(&m_endDate) << endl;
    }
    *fs << "      WaterVolume: " << m_waterVolume << ", Sediment: " << m_sedimentConc <<
        ", TN: " << m_TNConc << ", NO3: " << m_NO3Conc <<
        ", NH4: " << m_NH4Conc << ", OrgN: " << m_OrgNConc <<
        ", TP: " << m_TPConc << ", MinP: " << m_SolPConc <<
        ", OrgP: " << m_OrgPConc << endl;
}


/************************************************************************/
/*                      PointSourceLocations                                           */
/************************************************************************/

PointSourceLocations::PointSourceLocations(const bson_t *&bsonTable, bson_iter_t &iter)
    : m_size(0.f), m_distDown(0.f) {
    if (bson_iter_init_find(&iter, bsonTable, BMP_FLD_NAME)) {
        m_name = GetStringFromBsonIterator(&iter);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_PTSRCID)) {
        GetNumericFromBsonIterator(&iter, m_pointSrcID);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_LAT)) {
        GetNumericFromBsonIterator(&iter, m_lat);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_LON)) {
        GetNumericFromBsonIterator(&iter, m_lon);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_LOCALX)) {
        GetNumericFromBsonIterator(&iter, m_localX);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_LOCALY)) {
        GetNumericFromBsonIterator(&iter, m_localY);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_SUBBSN)) {
        GetNumericFromBsonIterator(&iter, m_subbasinID);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_SIZE)) {
        GetNumericFromBsonIterator(&iter, m_size);
    }
    if (bson_iter_init_find(&iter, bsonTable, BMP_PTSRC_FLD_DISTDOWN)) {
        GetNumericFromBsonIterator(&iter, m_distDown);
    }
}

PointSourceLocations::~PointSourceLocations(void) {
}

void PointSourceLocations::Dump(ostream *fs) {
    if (fs == NULL) return;
    *fs << "      Point Source Location: " << endl <<
        "        PTSRCID: " << m_pointSrcID << ", SubBasinID: " << m_subbasinID <<
        ", Lon: " << m_lon << ", Lat: " << m_lat <<
        ", LocalX: " << m_localX << ", LocalY: " << m_localY <<
        ", Size: " << m_size << ", DistanceDown: " << m_distDown << endl;
}
