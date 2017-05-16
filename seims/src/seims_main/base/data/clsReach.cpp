#include "clsReach.h"

using namespace std;

clsReach::clsReach(const bson_t *&bsonTable) {
    bson_iter_t iterator;
    /// reset default values
    Reset();
    if (bson_iter_init_find(&iterator, bsonTable, REACH_SUBBASIN)) {
        GetNumericFromBsonIterator(&iterator, this->SubbasinID);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_DOWNSTREAM)) {
        GetNumericFromBsonIterator(&iterator, this->DownStream);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_UPDOWN_ORDER)) {
        GetNumericFromBsonIterator(&iterator, this->UpDownOrder);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_DOWNUP_ORDER)) {
        GetNumericFromBsonIterator(&iterator, this->DownUpOrder);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_WIDTH)) {
        GetNumericFromBsonIterator(&iterator, this->Width);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_SIDESLP)) {
        GetNumericFromBsonIterator(&iterator, this->SideSlope);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_LENGTH)) {
        GetNumericFromBsonIterator(&iterator, this->Length);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_DEPTH)) {
        GetNumericFromBsonIterator(&iterator, this->Depth);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_V0)) {
        GetNumericFromBsonIterator(&iterator, this->V0);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_AREA)) {
        GetNumericFromBsonIterator(&iterator, this->Area);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_MANNING)) {
        GetNumericFromBsonIterator(&iterator, this->Manning);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_SLOPE)) {
        GetNumericFromBsonIterator(&iterator, this->Slope);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_GROUP)) {
        GetNumericFromBsonIterator(&iterator, this->Group);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_GROUPDIVIDED)) {
        GetNumericFromBsonIterator(&iterator, this->GroupDivided);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_NUMCELLS)) {
        GetNumericFromBsonIterator(&iterator, this->NumCells);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_BC1)) {
        GetNumericFromBsonIterator(&iterator, this->bc1);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_BC2)) {
        GetNumericFromBsonIterator(&iterator, this->bc2);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_BC3)) {
        GetNumericFromBsonIterator(&iterator, this->bc3);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_BC4)) {
        GetNumericFromBsonIterator(&iterator, this->bc4);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RS1)) {
        GetNumericFromBsonIterator(&iterator, this->rs1);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RS2)) {
        GetNumericFromBsonIterator(&iterator, this->rs2);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RS3)) {
        GetNumericFromBsonIterator(&iterator, this->rs3);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RS4)) {
        GetNumericFromBsonIterator(&iterator, this->rs4);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RS5)) {
        GetNumericFromBsonIterator(&iterator, this->rs5);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RK1)) {
        GetNumericFromBsonIterator(&iterator, this->rk1);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RK2)) {
        GetNumericFromBsonIterator(&iterator, this->rk2);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RK3)) {
        GetNumericFromBsonIterator(&iterator, this->rk3);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_RK4)) {
        GetNumericFromBsonIterator(&iterator, this->rk4);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_COVER)) {
        GetNumericFromBsonIterator(&iterator, this->cover);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_EROD)) {
        GetNumericFromBsonIterator(&iterator, this->erod);
    }
    /// nutrient related
    if (bson_iter_init_find(&iterator, bsonTable, REACH_DISOX)) {
        GetNumericFromBsonIterator(&iterator, this->disox);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_BOD)) {
        GetNumericFromBsonIterator(&iterator, this->cod);
        if (this->cod <= 1.e-6f) this->cod = 1.e-6f;
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_ALGAE)) {
        GetNumericFromBsonIterator(&iterator, this->algae);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_NO3)) {
        GetNumericFromBsonIterator(&iterator, this->no3);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_NO2)) {
        GetNumericFromBsonIterator(&iterator, this->no2);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_NH4)) {
        GetNumericFromBsonIterator(&iterator, this->nh4);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_ORGN)) {
        GetNumericFromBsonIterator(&iterator, this->orgn);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_ORGP)) {
        GetNumericFromBsonIterator(&iterator, this->orgp);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_SOLP)) {
        GetNumericFromBsonIterator(&iterator, this->solp);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_GWNO3)) {
        GetNumericFromBsonIterator(&iterator, this->gwno3);
    }
    if (bson_iter_init_find(&iterator, bsonTable, REACH_GWSOLP)) {
        GetNumericFromBsonIterator(&iterator, this->gwsolp);
    }
}

clsReach::~clsReach(void) {
}

void clsReach::Reset(void) {
    Area = NODATA_VALUE;
    Depth = NODATA_VALUE;
    DownStream = -1;
    DownUpOrder = -1;
    Group = -1;
    GroupDivided = -1;
    Length = NODATA_VALUE;
    Manning = NODATA_VALUE;
    NumCells = -1;
    Slope = NODATA_VALUE;
    SubbasinID = -1;
    UpDownOrder = -1;
    V0 = NODATA_VALUE;
    Width = NODATA_VALUE;
    SideSlope = 2.f;
    bc1 = 0.55f;
    bc2 = 1.1f;
    bc3 = 0.21f;
    bc4 = 0.35f;
    rk1 = 1.71f;
    rk2 = 50.f;
    rk3 = 0.36f;
    rk4 = 2.f;
    rs1 = 1.f;
    rs2 = 0.05f;
    rs3 = 0.5f;
    rs4 = 0.05f;
    rs5 = 0.05f;
}

clsReaches::clsReaches(MongoClient* conn, string &dbName, string collectionName) {
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
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    const bson_t *bsonTable;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable)) {
        clsReach *curReach = new clsReach(bsonTable);
        m_reachesMap[curReach->GetSubbasinID()] = curReach;
        this->m_reachIDs.push_back(curReach->GetSubbasinID());
    }
    vector<int>(m_reachIDs).swap(m_reachIDs);

    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

clsReaches::~clsReaches() {
    StatusMessage("Release clsReach...");
    if (!m_reachesMap.empty()) {
        for (map<int, clsReach *>::iterator iter = m_reachesMap.begin(); iter != m_reachesMap.end();) {
            if (iter->second != NULL) {
                delete iter->second;
                iter->second = NULL;
            }
            iter = m_reachesMap.erase(iter);
        }
        m_reachesMap.clear();
    }
}
