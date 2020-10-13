#include "clsReach.h"

#include "utils_array.h"
#include "utils_string.h"
#include "utils_math.h"

#include "text.h"
#include "ParamInfo.h"
//#include "Logging.h"

using namespace utils_array;
using namespace utils_string;
using namespace utils_math;

const int REACH_PARAM_NUM = 48; /// Numerical parameters, except GROUP related
const char* REACH_PARAM_NAME[] = {
    REACH_SUBBASIN, REACH_NUMCELLS,                           // 0-1
    REACH_DOWNSTREAM, REACH_UPDOWN_ORDER, REACH_DOWNUP_ORDER, // 2-4
    REACH_WIDTH, REACH_LENGTH, REACH_DEPTH, REACH_AREA,       // 5-8
    REACH_SIDESLP, REACH_SLOPE,                               // 9-10
    REACH_MANNING, REACH_BEDK, REACH_BNKK,                    // 11-13
    REACH_BEDBD, REACH_BNKBD, REACH_BEDCOV, REACH_BNKCOV,     // 14-17
    REACH_BEDEROD, REACH_BNKEROD, REACH_BEDD50, REACH_BNKD50, // 18-21
    REACH_BC1, REACH_BC2, REACH_BC3, REACH_BC4,               // 22-25
    REACH_RS1, REACH_RS2, REACH_RS3, REACH_RS4, REACH_RS5,    // 26-30
    REACH_RK1, REACH_RK2, REACH_RK3, REACH_RK4,               // 31-34
    REACH_DISOX, REACH_BOD, REACH_ALGAE,                      // 35-37
    REACH_ORGN, REACH_NH4, REACH_NO2, REACH_NO3,              // 38-41
    REACH_ORGP, REACH_SOLP, REACH_GWNO3, REACH_GWSOLP,        // 42-45
    REACH_BEDTC, REACH_BNKTC                                  // 46-47
};
const int REACH_GROUP_METHOD_NUM = 2; /// Group methods
const char* REACH_GROUP_NAME[] = {REACH_KMETIS, REACH_PMETIS};

clsReach::clsReach(const bson_t*& bson_table) {
    bson_iter_t iterator;
    // Read numerical parameters and set default values if not exists.
    // Note: The check of maximum and minimum values will be done in `clsReaches::Update`.
    for (int i = 0; i < REACH_PARAM_NUM; i++) {
        float tmp_param;
        if (bson_iter_init_find(&iterator, bson_table, REACH_PARAM_NAME[i]) &&
            GetNumericFromBsonIterator(&iterator, tmp_param)) {
            // Existed in database and is numerical value
            if (StringMatch(REACH_PARAM_NAME[i], REACH_BOD) && tmp_param < 1.e-6f) tmp_param = 1.e-6f;
        } else {
            // Not existed in database, the set default values
            if (StringMatch(REACH_PARAM_NAME[i], REACH_BEDTC)) tmp_param = 0.f;
            if (StringMatch(REACH_PARAM_NAME[i], REACH_BNKTC)) tmp_param = 0.f;
        }
#ifdef HAS_VARIADIC_TEMPLATES
        param_map_.emplace(REACH_PARAM_NAME[i], tmp_param);
#else
        param_map_.insert(make_pair(REACH_PARAM_NAME[i], tmp_param));
#endif
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
        for (size_t j = 0; j < group_number_.size(); j++) {
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

int clsReach::GetGroupIndex(const string& method, const int size) {
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


void clsReach::DerivedParameters() {
    float bnksize = 0.f; // Unit: millimeters
    float fr_sand = 0.f;
    float fr_silt = 0.f;
    float fr_clay = 0.f;
    float fr_gravel = 0.f;
    float tc = 0.f; // An estimate of Critical shear stress if it is not given (N/m^2), Julian and Torres(2005)
    float kd = 0.f;
    // An estimate of channel bank erodibility coefficient if it is not available, Hanson and Simon(2001)

    ////////// Bank related //////////
    if (param_map_.find(REACH_BNKD50) == param_map_.end()) {
        bnksize = 0.05f;
#ifdef HAS_VARIADIC_TEMPLATES
        param_map_.emplace(REACH_BNKD50, 50.f);
#else
        param_map_.insert(make_pair(REACH_BNKD50, 50.f));
#endif
    } else {
        bnksize = param_map_.at(REACH_BNKD50) / 1000.f;
    }
    // Channel bank sediment particle size distribution
    if (bnksize <= 0.005f) {
        // clayey bank
        fr_clay = 0.65f;
        fr_silt = 0.15f;
        fr_sand = 0.15f;
        fr_gravel = 0.05f;
    } else if (bnksize > 0.005f && bnksize <= 0.05f) {
        // silty bank
        fr_silt = 0.65f;
        fr_clay = 0.15f;
        fr_sand = 0.15f;
        fr_gravel = 0.05f;
    } else if (bnksize > 0.05f && bnksize <= 2.f) {
        // sand bank
        fr_sand = 0.65f;
        fr_silt = 0.15f;
        fr_clay = 0.15f;
        fr_gravel = 0.05f;
    } else {
        // gravel bank
        fr_gravel = 0.65f;
        fr_sand = 0.15f;
        fr_silt = 0.15f;
        fr_clay = 0.05f;
    }
#ifdef HAS_VARIADIC_TEMPLATES
    param_map_.emplace(REACH_BNKSAND, fr_sand);
    param_map_.emplace(REACH_BNKSILT, fr_silt);
    param_map_.emplace(REACH_BNKCLAY, fr_clay);
    param_map_.emplace(REACH_BNKGRAVEL, fr_gravel);
#else
    param_map_.insert(make_pair(REACH_BNKSAND, fr_sand));
    param_map_.insert(make_pair(REACH_BNKSILT, fr_silt));
    param_map_.insert(make_pair(REACH_BNKCLAY, fr_clay));
    param_map_.insert(make_pair(REACH_BNKGRAVEL, fr_gravel));
#endif
    // Critical shear stress of channel bank
    if (param_map_.find(REACH_BNKTC) != param_map_.end()) {
        tc = param_map_.at(REACH_BNKTC);
    }
    if (tc <= UTIL_ZERO) {
        float sc = (fr_silt + fr_clay) * 100.f;
        tc = (0.1f + 0.1779f * sc + 0.0028f * sc * sc - 2.34e-05f * sc * sc * sc) * param_map_.at(REACH_BNKCOV);
        if (param_map_.find(REACH_BNKTC) != param_map_.end()) {
            param_map_.at(REACH_BNKTC) = tc;
        } else {
#ifdef HAS_VARIADIC_TEMPLATES
            param_map_.emplace(REACH_BNKTC, tc);
#else
            param_map_.insert(make_pair(REACH_BNKTC, tc));
#endif
        }
    }
    // Erodibility coefficient of channel bank
    if (param_map_.find(REACH_BNKEROD) != param_map_.end()) {
        kd = param_map_.at(REACH_BNKEROD);
    }
    if (kd <= UTIL_ZERO) {
        if (tc <= UTIL_ZERO) {
            kd = 0.2;
        } else {
            kd = 0.2f / sqrt(tc);
        }
        if (param_map_.find(REACH_BNKEROD) != param_map_.end()) {
            param_map_.at(REACH_BNKEROD) = kd;
        } else {
#ifdef HAS_VARIADIC_TEMPLATES
            param_map_.emplace(REACH_BNKEROD, kd);
#else
            param_map_.insert(make_pair(REACH_BNKEROD, kd));
#endif
        }
    }

    ////////// Bed related //////////
    if (param_map_.find(REACH_BEDD50) == param_map_.end()) {
        bnksize = 0.5f;
#ifdef HAS_VARIADIC_TEMPLATES
        param_map_.emplace(REACH_BEDD50, 500.f);
#else
        param_map_.insert(make_pair(REACH_BEDD50, 500.f));
#endif
    } else {
        bnksize = param_map_.at(REACH_BEDD50) / 1000.f;
    }
    // Channel bank sediment particle size distribution
    if (bnksize <= 0.005f) {
        // clayey bank
        fr_clay = 0.65f;
        fr_silt = 0.15f;
        fr_sand = 0.15f;
        fr_gravel = 0.05f;
    } else if (bnksize > 0.005f && bnksize <= 0.05f) {
        // silty bank
        fr_silt = 0.65f;
        fr_clay = 0.15f;
        fr_sand = 0.15f;
        fr_gravel = 0.05f;
    } else if (bnksize > 0.05f && bnksize <= 2.f) {
        // sand bank
        fr_sand = 0.65f;
        fr_silt = 0.15f;
        fr_clay = 0.15f;
        fr_gravel = 0.05f;
    } else {
        // gravel bank
        fr_gravel = 0.65f;
        fr_sand = 0.15f;
        fr_silt = 0.15f;
        fr_clay = 0.05f;
    }
#ifdef HAS_VARIADIC_TEMPLATES
    param_map_.emplace(REACH_BEDSAND, fr_sand);
    param_map_.emplace(REACH_BEDSILT, fr_silt);
    param_map_.emplace(REACH_BEDCLAY, fr_clay);
    param_map_.emplace(REACH_BEDGRAVEL, fr_gravel);
#else
    param_map_.insert(make_pair(REACH_BEDSAND, fr_sand));
    param_map_.insert(make_pair(REACH_BEDSILT, fr_silt));
    param_map_.insert(make_pair(REACH_BEDCLAY, fr_clay));
    param_map_.insert(make_pair(REACH_BEDGRAVEL, fr_gravel));
#endif
    // Critical shear stress of channel bed
    if (param_map_.find(REACH_BEDTC) != param_map_.end()) {
        tc = param_map_.at(REACH_BEDTC);
    }
    if (tc <= UTIL_ZERO) {
        float sc = (fr_silt + fr_clay) * 100.f;
        tc = (0.1f + 0.1779f * sc + 0.0028f * sc * sc - 2.34e-05f * sc * sc * sc) * param_map_.at(REACH_BEDCOV);
        if (param_map_.find(REACH_BEDTC) != param_map_.end()) {
            param_map_.at(REACH_BEDTC) = tc;
        } else {
#ifdef HAS_VARIADIC_TEMPLATES
            param_map_.emplace(REACH_BEDTC, tc);
#else
            param_map_.insert(make_pair(REACH_BEDTC, tc));
#endif
        }
    }
    // Erodibility coefficient of channel bed
    if (param_map_.find(REACH_BEDEROD) != param_map_.end()) {
        kd = param_map_.at(REACH_BEDEROD);
    }
    if (kd <= UTIL_ZERO) {
        if (tc <= UTIL_ZERO) {
            kd = 0.2;
        } else {
            kd = 0.2f / sqrt(tc);
        }
        if (param_map_.find(REACH_BEDEROD) != param_map_.end()) {
            param_map_.at(REACH_BEDEROD) = kd;
        } else {
#ifdef HAS_VARIADIC_TEMPLATES
            param_map_.emplace(REACH_BEDEROD, kd);
#else
            param_map_.insert(make_pair(REACH_BEDEROD, kd));
#endif
        }
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
        int sub_id = CVT_INT(cur_reach->Get(REACH_SUBBASIN));
#ifdef HAS_VARIADIC_TEMPLATES
        reaches_obj_.emplace(sub_id, cur_reach);
#else
        reaches_obj_.insert(make_pair(sub_id, cur_reach));
#endif
    }
    // In SEIMS, reach ID is the same as Index of array and vector.
    for (int i = 1; i <= reach_num_; i++) {
        int down_stream_id = CVT_INT(reaches_obj_.at(i)->Get(REACH_DOWNSTREAM));
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
        int order = mtd == UP_DOWN
                        ? CVT_INT(reaches_obj_.at(i)->Get(REACH_UPDOWN_ORDER))
                        : CVT_INT(reaches_obj_.at(i)->Get(REACH_DOWNUP_ORDER));
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
    //CLOG(TRACE, LOG_RELEASE) << "Release clsReach...";
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
            // There is no need to check the CHANGE types, since the `GetAdjustedValue` will cover this!
            //ParamInfo* tmp_param = it->second;
            //if ((StringMatch(tmp_param->Change, PARAM_CHANGE_RC) && FloatEqual(tmp_param->Impact, 1.f)) ||
            //    (StringMatch(tmp_param->Change, PARAM_CHANGE_AC) && FloatEqual(tmp_param->Impact, 0.f)) ||
            //    (StringMatch(tmp_param->Change, PARAM_CHANGE_VC) && FloatEqual(tmp_param->Impact, NODATA_VALUE)) ||
            //    StringMatch(tmp_param->Change, PARAM_CHANGE_NC)) {
            //    continue;
            //}
            for (auto it2 = reaches_obj_.begin(); it2 != reaches_obj_.end(); ++it2) {
                float pre_value = it2->second->Get(REACH_PARAM_NAME[i]);
                if (FloatEqual(pre_value, NODATA_VALUE)) continue;
                it2->second->Set(REACH_PARAM_NAME[i], it->second->GetAdjustedValue(pre_value));
            }
        }
    }
    // Calculate derived parameters, and some specifications
    for (auto itrch = reaches_obj_.begin(); itrch != reaches_obj_.end(); ++itrch) {
        itrch->second->DerivedParameters();
        // Update width-depth-ratio according to reach width and reach depth.
        if (itrch->second->Get(REACH_DEPTH) <= UTIL_ZERO) {
            cout << "WARNING: Reach ID " << itrch->first << " has a negative depth, set to default 1.5!" << endl;
            itrch->second->Set(REACH_DEPTH, 1.5f);
        }
        if (itrch->second->Get(REACH_WIDTH) <= UTIL_ZERO) {
            cout << "WARNING: Reach ID " << itrch->first << " has a negative width, set to default 5!" << endl;
            itrch->second->Set(REACH_WIDTH, 5.f);
        }
        itrch->second->Set(REACH_WDRATIO, itrch->second->Get(REACH_WIDTH) / itrch->second->Get(REACH_DEPTH));
    }
}
