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
#include "utils_array.h"
#include "db_mongoc.h"

using namespace ccgl;
using namespace utils_array;
using namespace db_mongoc;
using std::map;
using std::set;

/*!
 * \brief Simple struct of subbasin information for task allocation
 */
class SubbasinStruct: NotCopyable {
public:
    SubbasinStruct(int sid, int gidx);
    ~SubbasinStruct();
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
