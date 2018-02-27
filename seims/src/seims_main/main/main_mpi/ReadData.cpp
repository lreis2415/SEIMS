#include "ReadData.h"

#include "invoke.h"

using namespace std;

int CreateReachTopology(MongoClient *client, string &dbname, string &group_method, int group_size,
                        map<int, SubbasinStruct*> &subbasins, set<int> &group_set) {
    // Read reach information from MongoDB Collection "REACHES"
    clsReaches *reaches = new clsReaches(client, dbname, DB_TAB_REACH);
    // Get downstream map
    map<int, int> downStreamMap = reaches->GetDownStreamID();
    // Create SubbasinStruct's map and group_set
    for (auto it = downStreamMap.begin(); it != downStreamMap.end(); it++) {
        int id = it->first;
        clsReach *tmp_reach = reaches->GetReachByID(id);
        int group = tmp_reach->GetGroupIndex(group_method, group_size);
        subbasins[id] = new SubbasinStruct(id, group);
        // set layering order
        subbasins[id]->updown_order = (int) tmp_reach->Get(REACH_UPDOWN_ORDER);
        subbasins[id]->downup_order = (int) tmp_reach->Get(REACH_DOWNUP_ORDER);

        group_set.insert(group);
    }
    // fill topology information
    for (auto it = downStreamMap.begin(); it != downStreamMap.end(); it++) {
        int id = it->first;
        int to = it->second;
        if (to > 0) {
            subbasins[id]->downStream = subbasins[to];
            subbasins[to]->upStreams.push_back(subbasins[id]);
        }
    }
#ifdef _DEBUG
    StatusMessage("---CreateReachTopology done---");
    cout << "Group set: ";
    for (auto it = group_set.begin(); it != group_set.end(); it++)
        cout << *it << " ";
    cout << endl;
    cout << "Subbasin size: " << subbasins.size() << endl;
    cout << "SubbasinStruct Map: " << endl;
    for (auto it = subbasins.begin(); it != subbasins.end(); it++) {
        cout << "  Subbasin ID: " << it->first;
        cout << ", Group index: " << it->second->group;
        cout << ", Updown order: " << it->second->updown_order;
        cout << ", Downup order: " << it->second->updown_order;
        if (nullptr == it->second->downStream)
            cout << ", Downstream ID: None";
        else
            cout << ", Downstream ID: " << it->second->downStream->id;
        cout << endl;
    }
    cout << endl;
#endif /* _DEBUG */
    delete reaches;
    return 0;
}
