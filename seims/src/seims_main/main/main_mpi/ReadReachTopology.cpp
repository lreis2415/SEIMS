#include "ReadReachTopology.h"
#include "parallel.h"

#include "text.h"
#include "clsReach.h"
#include "Logging.h"

SubbasinStruct::SubbasinStruct(const int sid, const int gidx) :
    id(sid), group(gidx),
    updown_order(-1), downup_order(-1), calculated(false),
    transfer_count(-1), transfer_values(nullptr),
    down_stream(nullptr) {
    up_streams.clear();
}

SubbasinStruct::~SubbasinStruct() {
    if (transfer_values != nullptr) { Release1DArray(transfer_values); }
    if (!up_streams.empty()) {
        for (auto it = up_streams.begin(); it != up_streams.end();) {
            if (*it != nullptr) *it = nullptr;
            it = up_streams.erase(it);
        }
    }
    if (down_stream != nullptr) down_stream = nullptr;
}


int CreateReachTopology(MongoClient* client, const string& dbname,
                        GroupMethod group_method, const int group_size,
                        map<int, SubbasinStruct *>& subbasins, set<int>& group_set) {
    // Read reach information from MongoDB Collection "REACHES"
    clsReaches* reaches = new clsReaches(client, dbname, DB_TAB_REACH);
    // Get downstream map
    map<int, int>& down_stream_map = reaches->GetDownStreamID();
    // Create SubbasinStruct's map and group_set
    for (auto it = down_stream_map.begin(); it != down_stream_map.end(); ++it) {
        int id = it->first;
        clsReach* tmp_reach = reaches->GetReachByID(id);
        int group = tmp_reach->GetGroupIndex(GroupMethodString[CVT_INT(group_method)], group_size);
        subbasins[id] = new SubbasinStruct(id, group);
        // set layering order
        subbasins[id]->updown_order = CVT_INT(tmp_reach->Get(REACH_UPDOWN_ORDER));
        subbasins[id]->downup_order = CVT_INT(tmp_reach->Get(REACH_DOWNUP_ORDER));

        group_set.insert(group);
    }
    // fill topology information
    for (auto it = down_stream_map.begin(); it != down_stream_map.end(); ++it) {
        int id = it->first;
        int to = it->second;
        if (to > 0) {
            subbasins[id]->down_stream = subbasins[to];
            subbasins[to]->up_streams.emplace_back(subbasins[id]);
        }
    }
    CLOG(TRACE, LOG_INIT) << "---CreateReachTopology done---";
    std::ostringstream oss;
    oss << "Group set: ";
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        oss << *it << " ";
    }
    CLOG(TRACE, LOG_INIT) << oss.str();
    CLOG(TRACE, LOG_INIT) << "Subbasin size: " << subbasins.size();
    CLOG(TRACE, LOG_INIT) << "SubbasinStruct Map: ";
    for (auto it = subbasins.begin(); it != subbasins.end(); ++it) {
        std::ostringstream oss2;
        oss2 << "  Subbasin ID: " << it->first << ", Group index: " << it->second->group
                << ", Updown order: " << it->second->updown_order
                << ", Downup order: " << it->second->updown_order;
        if (nullptr == it->second->down_stream) {
            oss2 << ", Downstream ID: None";
        } else {
            oss2 << ", Downstream ID: " << it->second->down_stream->id;
        }
        CLOG(TRACE, LOG_INIT) << oss2.str();
    }

    delete reaches;
    return 0;
}
