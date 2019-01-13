/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
#include "clsPond.h"

#include "utils_array.h"
#include "utils_string.h"
#include "utils_math.h"

#include "text.h"
#include "ParamInfo.h"

using namespace utils_array;
using namespace utils_string;
using namespace utils_math;

const int POND_PARAM_NUM = 5;
const char* POND_PARAM_NAME[] = {
    POND_PADDYID, POND_REACHID, POND_PONDID1, POND_PONDID2, POND_PONDID3
};

clsPond::clsPond(const bson_t*& bson_table)
{
	bson_iter_t iterator;
    for (int i = 0; i < POND_PARAM_NUM; i++) {
        float tmp_param;
        if (bson_iter_init_find(&iterator, bson_table, POND_PARAM_NAME[i])){
            if (GetNumericFromBsonIterator(&iterator, tmp_param)) {
                //if (tmp_param < 1.e-6f) tmp_param = 1.e-6f;
                param_map_.emplace(POND_PARAM_NAME[i], tmp_param);
            }
        }
    }
	/*if (bson_iter_init_find(&iterator, bsonTable, POND_PADDYID))
		GetNumericFromBsonIterator(&iterator, this->PaddyID);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_REACHID))
		GetNumericFromBsonIterator(&iterator, this->ReachID);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID1))
		GetNumericFromBsonIterator(&iterator, this->PondID1);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID2))
		GetNumericFromBsonIterator(&iterator, this->PondID2);
		
	if (bson_iter_init_find(&iterator, bsonTable, POND_PONDID3))
		GetNumericFromBsonIterator(&iterator, this->PondID3);*/
		
}

float clsPond::Get(const string& key) {
    auto it = param_map_.find(key);
    if (it != param_map_.end()) {
        return it->second;
    }
    return NODATA_VALUE;
}

void clsPond::Set(const string& key, const float value) {
    auto it = param_map_.find(key);
    if (it != param_map_.end()) {
        param_map_.at(key) = value;
    }
    else {
#ifdef HAS_VARIADIC_TEMPLATES
        param_map_.emplace(key, NODATA_VALUE);
#else
        param_map_.insert(make_pair(key, NODATA_VALUE));
#endif
    }
}

//clsPond::~clsPond(void)
//{
//}

clsPonds::clsPonds(MongoClient* conn, const string& db_name, const string& collection_name, const LayeringMethod mtd){
    bson_t* b = bson_new();
    bson_t* child1 = bson_new();
    bson_t* child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);  /// query all records
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2); /// and order by pond ID
    BSON_APPEND_INT32(child2, POND_PADDYID, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    std::unique_ptr<MongoCollection> collection(new MongoCollection(conn->GetCollection(db_name, collection_name)));
    paddy_num_ = collection->QueryRecordsCount();
    if (paddy_num_ < 0) {
        bson_destroy(b);
        throw ModelException("clsReaches", "ReadAllReachInfo",
            "Failed to get document number of collection: " + collection_name + ".\n");
    }
    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    const bson_t* bson_table;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
        clsPond* cur_pond = new clsPond(bson_table);
        int paddy_id = CVT_INT(cur_pond->Get(POND_PADDYID));
        pondsInfo_.emplace(paddy_id, cur_pond);
        paddyIDs_.push_back(paddy_id);
    }

    /*while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table))
    {
        clsPond *curPond = new clsPond(bson_table);
        pondsInfo_[curPond->Get(POND_PADDYID)] = curPond;
        this->paddyIDs_.push_back(curPond->Get(POND_PADDYID));
    }
    vector<int>(paddyIDs_).swap(paddyIDs_);*/

    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

	/*mongoc_cursor_t *cursor;
	const bson_t *bsonTable;
	mongoc_collection_t *collection = NULL;
	bson_error_t *err = NULL;*/

	/*collection = mongoc_client_get_collection(conn, dbName.c_str(), collectionName.c_str());
	if (collection == NULL)
		throw ModelException("clsPonds", "ReadAllPondsInfo", "Failed to get collection: " + collectionName + ".\n");
	const bson_t *qCount = bson_new();
	this->m_paddyNum = (int) mongoc_collection_count(collection, MONGOC_QUERY_NONE, qCount, 0, 0, NULL, err);
	if (err != NULL || this->m_paddyNum < 0)
		throw ModelException("clsPonds", "ReadAllPondsInfo",
		"Failed to get document number of collection: " + collectionName + ".\n");
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);*/


	/*while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bsonTable))
	{
		clsPond *curPond = new clsPond(bsonTable);
		m_pondsInfo[curPond->GetPaddyID()] = curPond;
		this->m_paddyIDs.push_back(curPond->GetPaddyID());
	}
	vector<int>(m_paddyIDs).swap(m_paddyIDs);

	bson_destroy(b);
	mongoc_collection_destroy(collection);
	mongoc_cursor_destroy(cursor);
}*/
clsPonds::~clsPonds(void)
{
    if (!pondsInfo_.empty())
	{
        for (map<int, clsPond *>::iterator iter = pondsInfo_.begin(); iter != pondsInfo_.end();)
		{
			if (iter->second != NULL)
				delete iter->second;
            iter = pondsInfo_.erase(iter);
		}
        pondsInfo_.clear();
	}
}

void clsPonds::Update(map<string, ParamInfo *>& caliparams_map) {
    for (int i = 0; i < POND_PARAM_NUM; i++) {
        auto it = caliparams_map.find(POND_PARAM_NAME[i]);
        if (it != caliparams_map.end()) {
            ParamInfo* tmp_param = it->second;
            if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && FloatEqual(tmp_param->Impact, 1.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && FloatEqual(tmp_param->Impact, 0.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
                StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
                continue;
            }
            for (auto it2 = pondsInfo_.begin(); it2 != pondsInfo_.end(); ++it2) {
                float pre_value = it2->second->Get(POND_PARAM_NAME[i]);
                if (FloatEqual(pre_value, NODATA_VALUE)) continue;
                it2->second->Set(POND_PARAM_NAME[i], it->second->GetAdjustedValue(pre_value));
            }
        }
    }
}
