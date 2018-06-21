#include "clsReach.h"

#include "utils_array.h"
#include "utils_string.h"
#include "utils_math.h"

#include "text.h"
#include "ParamInfo.h"

using namespace utils_array;
using namespace utils_string;
using namespace utils_math;

const int REACH_PARAM_NUM = 41; /// Numerical parameters
const char* REACH_PARAM_NAME[] = {
    REACH_SUBBASIN, REACH_NUMCELLS,                           // 0-1
    REACH_DOWNSTREAM, REACH_UPDOWN_ORDER, REACH_DOWNUP_ORDER, // 2-4
    REACH_WIDTH, REACH_SIDESLP, REACH_LENGTH, REACH_DEPTH,    // 5-8
    REACH_V0, REACH_AREA, REACH_MANNING, REACH_SLOPE,         // 9-12
    REACH_KBANK, REACH_KBED, REACH_COVER, REACH_EROD,         // 13-16
    REACH_BC1, REACH_BC2, REACH_BC3, REACH_BC4,               // 17-20
    REACH_RS1, REACH_RS2, REACH_RS3, REACH_RS4, REACH_RS5,    // 21-25
    REACH_RK1, REACH_RK2, REACH_RK3, REACH_RK4,               // 26-29
    REACH_DISOX, REACH_BOD, REACH_ALGAE,                      // 30-32
    REACH_ORGN, REACH_NH4, REACH_NO2, REACH_NO3,              // 33-36
    REACH_ORGP, REACH_SOLP, REACH_GWNO3, REACH_GWSOLP         // 37-40
};
const int REACH_GROUP_METHOD_NUM = 2; /// Group methods
const char* REACH_GROUP_NAME[] = {REACH_KMETIS, REACH_PMETIS};

clsReach::clsReach(const bson_t*& bson_table) {
    bson_iter_t iterator;
    // read numerical parameters
    for (int i = 0; i < REACH_PARAM_NUM; i++) {
        float tmp_param;
        if (bson_iter_init_find(&iterator, bson_table, REACH_PARAM_NAME[i])) {
            if (GetNumericFromBsonIterator(&iterator, tmp_param)) {
                if (REACH_PARAM_NAME[i] == REACH_BOD && tmp_param < 1.e-6f) tmp_param = 1.e-6f;
#ifdef HAS_VARIADIC_TEMPLATES
                param_map_.emplace(REACH_PARAM_NAME[i], tmp_param);
#else
                param_map_.insert(make_pair(REACH_PARAM_NAME[i], tmp_param));
#endif
            }
        }
    }
    // read group informations, DO NOT THROW EXCEPTION!
    if (!bson_iter_init_find(&iterator, bson_table, REACH_GROUP)) {
        cout << "No GROUP field found!" << endl;
        return;
    }
    if (!SplitStringForValues(GetStringFromBsonIterator(&iterator), ',', group_number_)) {
        cout << "Split for group numbers failed!" << endl;
        return;
    }
    for (int i = 0; i < REACH_GROUP_METHOD_NUM; i++) {
        if (!bson_iter_init_find(&iterator, bson_table, REACH_GROUP_NAME[i])) continue;
        vector<int> g_idx;
        if (!SplitStringForValues(GetStringFromBsonIterator(&iterator), ',', g_idx)) {
            cout << "Split for group indexes failed!" << endl;
            return;
        }
        if (group_number_.size() != g_idx.size()) {
            cout << "The size of group method " << REACH_GROUP_NAME[i] << " is not equal to GROUP! " << endl;
            continue;
        }
        map<int, int> group_index;
        for (int j = 0; j < group_number_.size(); j++) {
#ifdef HAS_VARIADIC_TEMPLATES
            group_index.emplace(group_number_[j], g_idx[j]);
#else
            group_index.insert(make_pair(group_number_[j], g_idx[j]));
#endif
        }
        if (group_index.empty()) continue;
#ifdef HAS_VARIADIC_TEMPLATES
        group_index_.emplace(REACH_GROUP_NAME[i], group_index);
#else
        group_index_.insert(make_pair(REACH_GROUP_NAME[i], group_index));
#endif
    }
}

float clsReach::Get(const string& key) {
    auto it = param_map_.find(key);
    if (it != param_map_.end()) {
        return it->second;
    }
    return NODATA_VALUE;
}

int clsReach::GetGroupIndex(const string& method, int size) {
    if (group_index_.empty()) return -1;
    if (group_index_.find(method) == group_index_.end()) return -1;
    map<int, int> tmp = group_index_.at(method);
    if (tmp.find(size) == tmp.end()) return -1;
    return tmp.at(size);
}

void clsReach::Set(const string& key, const float value) {
    auto it = param_map_.find(key);
    if (it != param_map_.end()) {
        param_map_.at(key) = value;
    } else {
#ifdef HAS_VARIADIC_TEMPLATES
        param_map_.emplace(key, NODATA_VALUE);
#else
        param_map_.insert(make_pair(key, NODATA_VALUE));
#endif
    }
}

clsReaches::clsReaches(MongoClient* conn, const string& db_name,
                       const string& collection_name, const LayeringMethod mtd /* = UP_DOWN */) {
    bson_t* b = bson_new();
    bson_t* child1 = bson_new();
    bson_t* child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1); /// query all records
    bson_append_document_end(b, child1);
    BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2); /// and order by subbasin ID
    BSON_APPEND_INT32(child2, REACH_SUBBASIN, 1);
    bson_append_document_end(b, child2);
    bson_destroy(child1);
    bson_destroy(child2);

    std::unique_ptr<MongoCollection> collection(new MongoCollection(conn->GetCollection(db_name, collection_name)));
    reach_num_ = collection->QueryRecordsCount();
    if (reach_num_ < 0) {
        bson_destroy(b);
        // TODO, No exception should be thrown during constructing a class, considering Init function. lj.
        throw ModelException("clsReaches", "ReadAllReachInfo",
                             "Failed to get document number of collection: " + collection_name + ".\n");
    }
    reach_up_streams_.resize(reach_num_ + 1);

    mongoc_cursor_t* cursor = collection->ExecuteQuery(b);
    const bson_t* bson_table;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &bson_table)) {
        clsReach* cur_reach = new clsReach(bson_table);
        int sub_id = int(cur_reach->Get(REACH_SUBBASIN));
#ifdef HAS_VARIADIC_TEMPLATES
        reaches_obj_.emplace(sub_id, cur_reach);
#else
        reaches_obj_.insert(make_pair(sub_id, cur_reach));
#endif
    }
    // In SEIMS, reach ID is the same as Index of array and vector.
    for (int i = 1; i <= reach_num_; i++) {
        int down_stream_id = int(reaches_obj_.at(i)->Get(REACH_DOWNSTREAM));
#ifdef HAS_VARIADIC_TEMPLATES
        reach_down_stream_.emplace(i, down_stream_id);
#else
        reach_down_stream_.insert(make_pair(i, down_stream_id));
#endif
        if (down_stream_id <= 0 || down_stream_id > reach_num_) continue;
        reach_up_streams_[down_stream_id].emplace_back(i);
    }
    // Build layers of reaches according to layering method
    reach_layers_.clear();
    for (int i = 1; i <= reach_num_; i++) {
        int order = int(reaches_obj_.at(i)->Get(REACH_UPDOWN_ORDER));
        if (mtd == DOWN_UP) { order = int(reaches_obj_.at(i)->Get(REACH_DOWNUP_ORDER)); }
        if (reach_layers_.find(order) == reach_layers_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
            reach_layers_.emplace(order, vector<int>());
#else
            reach_layers_.insert(make_pair(order, vector<int>()));
#endif
        }
        reach_layers_.at(order).emplace_back(i);
    }

    bson_destroy(b);
    mongoc_cursor_destroy(cursor);
}

clsReaches::~clsReaches() {
    StatusMessage("Release clsReach...");
    if (!reaches_obj_.empty()) {
        for (auto iter = reaches_obj_.begin(); iter != reaches_obj_.end();) {
            if (nullptr != iter->second) {
                delete iter->second;
                iter->second = nullptr;
            }
            reaches_obj_.erase(iter++);
        }
        reaches_obj_.clear();
    }
    if (!reaches_properties_.empty()) {
        for (auto iter = reaches_properties_.begin(); iter != reaches_properties_.end();) {
            if (nullptr != iter->second) {
                Release1DArray(iter->second);
            }
            reaches_properties_.erase(iter++);
        }
        reaches_properties_.clear();
    }
}

void clsReaches::GetReachesSingleProperty(const string& key, float** data) {
    auto iter = reaches_properties_.find(key);
    if (iter == reaches_properties_.end()) {
        float* values = nullptr;
        Initialize1DArray(reach_num_ + 1, values, 0.f);
        values[0] = CVT_FLT(reach_num_);
        for (auto it = reaches_obj_.begin(); it != reaches_obj_.end(); ++it) {
            values[it->first] = it->second->Get(key);
        }
        *data = values;
    } else {
        *data = iter->second;
    }
}

void clsReaches::Update(map<string, ParamInfo *>& caliparams_map) {
    for (int i = 0; i < REACH_PARAM_NUM; i++) {
        auto it = caliparams_map.find(REACH_PARAM_NAME[i]);
        if (it != caliparams_map.end()) {
            ParamInfo* tmp_param = it->second;
            if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && FloatEqual(tmp_param->Impact, 1.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && FloatEqual(tmp_param->Impact, 0.f)) ||
                (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
                StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
                continue;
            }
            for (auto it2 = reaches_obj_.begin(); it2 != reaches_obj_.end(); ++it2) {
                float pre_value = it2->second->Get(REACH_PARAM_NAME[i]);
                if (FloatEqual(pre_value, NODATA_VALUE)) continue;
                it2->second->Set(REACH_PARAM_NAME[i], it->second->GetAdjustedValue(pre_value));
            }
        }
    }
}
