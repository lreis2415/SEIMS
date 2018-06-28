#include "ReadReachTopology.h"
#include "parallel.h"

#include "text.h"
#include "clsReach.h"

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
#ifdef _DEBUG
    StatusMessage("---CreateReachTopology done---");
    cout << "Group set: ";
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
    cout << "Subbasin size: " << subbasins.size() << endl;
    cout << "SubbasinStruct Map: " << endl;
    for (auto it = subbasins.begin(); it != subbasins.end(); ++it) {
        cout << "  Subbasin ID: " << it->first;
        cout << ", Group index: " << it->second->group;
        cout << ", Updown order: " << it->second->updown_order;
        cout << ", Downup order: " << it->second->updown_order;
        if (nullptr == it->second->down_stream) {
            cout << ", Downstream ID: None";
        } else {
            cout << ", Downstream ID: " << it->second->down_stream->id;
        }
        cout << endl;
    }
    cout << endl;
#endif /* _DEBUG */
    delete reaches;
    return 0;
}
