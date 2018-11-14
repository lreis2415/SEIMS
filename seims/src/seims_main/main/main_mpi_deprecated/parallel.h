/*!
 * \brief Header of MPI version of SEIMS framework
 * \author Junzhi Liu, Liangjun Zhu
 * \date 2018-03-20
 * \description  2018-03-20  lj  refactor as a more flexible framework to support various transferred variables.
 */
#ifndef SEIMS_MPI_H
#define SEIMS_MPI_H

#include <map>
#include <set>
#include <vector>
#include <string>

#ifdef MSVC
// Ignore warning on Windows MSVC compiler caused by MPI.
#pragma warning(disable: 4819)
#endif /* MSVC */
#include "mpi.h"

#include "basic.h"

#include "invoke.h"
#include "ModelMain.h"

#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1 ///< Rank of this slave processor in SlaveGroup is 0
#define MAX_UPSTREAM 4
#define MSG_LEN 5
#define MCW MPI_COMM_WORLD

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

    SubbasinStruct* down_stream;         ///< down stream subbasin
    vector<SubbasinStruct *> up_streams; ///< up stream subbasins
};

/*!
 * \brief Read reach table from MongoDB and create reach topology for task allocation.
 */
int CreateReachTopology(MongoClient* client, const string& dbname,
                        const string& group_method, int group_size,
                        map<int, SubbasinStruct *>& subbasins, set<int>& group_set);
/*!
 * \brief Management process
 * \param subbasin_map Map of all subbasins, used as transferred data repository
 * \param group_set Divided group ids, normally, 0 ~ N-1, the size equals to the number of slave processors
 * \return 0 for success
 */
int MasterProcess(map<int, SubbasinStruct *>& subbasin_map, set<int>& group_set);

/*!
 * \brief Calculation process
 * \param world_rank Rank number
 * \param numprocs Number of all processors, including one management rank and N-1 slave ranks
 * \param nslaves Number of calculation processors (also called slave ranks)
 * \param slave_comm MPI communicator used in slave group
 * \param input_args Input arguments
 */
void CalculateProcess(int world_rank, int numprocs, int nslaves, MPI_Comm slave_comm,
                      InputArgs* input_args);

#endif /* SEIMS_MPI_H */
