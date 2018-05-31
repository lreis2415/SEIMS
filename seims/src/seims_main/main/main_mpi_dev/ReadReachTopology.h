/*!
 * \brief Read and create reach (i.e., subbasin) topology data.
 * \author Junzhi Liu, Liangjun Zhu
 * \changelog  2018-03-20  - lj -  Refactor as a more flexible framework to
 *                                 support various transferred variables.
 */
#ifndef SEIMS_READ_REACH_TOPOLOGY_H
#define SEIMS_READ_REACH_TOPOLOGY_H

#include <map>
#include <set>
#include <vector>
#include <string>

#include "basic.h"

using std::map;
using std::set;
/*!
 * \brief Simple struct of subbasin information for task allocation
 */
class SubbasinStruct: NotCopyable {
public:
    SubbasinStruct(const int sid, const int gidx) : id(sid), group(gidx),
                                                    updown_order(-1), downup_order(-1), calculated(false),
                                                    transfer_count(-1), transfer_values(nullptr),
                                                    down_stream(nullptr) {
        up_streams.clear();
    }

    ~SubbasinStruct() {
        if (transfer_values != nullptr) { Release1DArray(transfer_values); }
        if (!up_streams.empty()) {
            for (auto it = up_streams.begin(); it != up_streams.end();) {
                if (*it != nullptr) *it = nullptr;
                it = up_streams.erase(it);
            }
        }
        if (down_stream != nullptr) down_stream = nullptr;
    };
public:
    int id;           ///< Subbasin ID, start from 1
    int group;        ///< Group index, start from 0 to (group number - 1)
    int updown_order; ///< up-down stream order
    int downup_order; ///< down-up stream order
    bool calculated;  ///< whether this subbasin is already calculated

    /****** Parameters need to transferred among subbasins *******/
    int transfer_count;     ///< count of transferred values
    float* transfer_values; ///< transferred values

    SubbasinStruct* down_stream;         ///< down stream subbasin \sa SubbasinStruct
    vector<SubbasinStruct *> up_streams; ///< up stream subbasins
};

/*!
 * \brief Read reach table from MongoDB and create reach topology for task allocation.
 */
int CreateReachTopology(MongoClient* client, const string& dbname,
                        const string& group_method, int group_size,
                        map<int, SubbasinStruct *>& subbasins, set<int>& group_set);

#endif /* SEIMS_READ_REACH_TOPOLOGY_H */
