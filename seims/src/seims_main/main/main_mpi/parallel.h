/*!
 * \brief Header of MPI version of SEIMS framework
 * \author Junzhi Liu, Liangjun Zhu
 * \date 2018-03-20
 * \description  2018-03-20  lj  refactor as a more flexible framework to support various transferred variables.
 */
#ifndef SEIMS_MPI_H
#define SEIMS_MPI_H

#include "seims.h"
#include "utilities.h"
#include "invoke.h"
#include "ModelMain.h"

#ifdef USE_MONGODB
#include "DataCenterMongoDB.h"

#endif /* USE_MONGODB */
#include "clsReach.h"

#include "mpi.h"
#include "mongoc.h"
#include "gdal.h"
#include "ogrsf_frmts.h"

#include <map>
#include <set>
#include <vector>
#include <string>

#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1 ///< Rank of this slave processor in SlaveGroup is 0
#define MAX_UPSTREAM 4
#define MSG_LEN 5
#define MCW MPI_COMM_WORLD

using namespace std;
/*!
 * \brief Simple struct of subbasin information for task allocation
 */
struct SubbasinStruct {
    SubbasinStruct(int sid, int gidx) : id(sid), group(gidx),
                                        updown_order(-1), downup_order(-1), calculated(false),
                                        transfer_count(-1), transfer_values(nullptr),
                                        downStream(nullptr) {
        upStreams.clear();
    }
    ~SubbasinStruct() {
        if (transfer_values != nullptr) { Release1DArray(transfer_values); }
        if (!upStreams.empty()) {
            for (auto it = upStreams.begin(); it != upStreams.end();) {
                if (*it != nullptr) *it = nullptr;
                it = upStreams.erase(it);
            }
        }
        if (downStream != nullptr) downStream = nullptr;
    };
    int id; ///< Subbasin ID, start from 1
    int group; ///< Group index, start from 0 to (group number - 1)

    int updown_order; ///< up-down stream order
    int downup_order; ///< down-up stream order
    bool calculated; ///< whether this subbasin is already calculated

    /****** Parameters need to transferred among subbasins *******/
    int transfer_count; ///< count of transferred values
    float *transfer_values; ///< transferred values

    SubbasinStruct *downStream; ///< down stream subbasin \sa SubbasinStruct
    vector<SubbasinStruct *> upStreams; ///< up stream subbasins
};

/*!
 * \brief Read reach table from MongoDB and create reach topology for task allocation.
 */
int CreateReachTopology(MongoClient *client, string &dbname, string &group_method, int group_size,
                        map<int, SubbasinStruct *> &subbasins, set<int> &group_set);
/*!
 * \brief Management process
 * \param subbasinMap Map of all subbasins, used as transferred data repository
 * \param groupSet Divided group ids, normally, 0 ~ N-1, the size equals to the number of slave processors
 * \return 0 for success
 */
int MasterProcess(map<int, SubbasinStruct *> &subbasinMap, set<int> &groupSet);

/*!
 * \brief Calculation process
 * \param world_rank Rank number
 * \param numprocs Number of all processors, including one management rank and N-1 slave ranks
 * \param nSlaves Number of calculation processors (also called slave ranks)
 * \param slaveComm MPI communicator used in slave group
 * \param input_args Input arguments, \sa InputArgs
 */
void CalculateProcess(int world_rank, int numprocs, int nSlaves, MPI_Comm slaveComm, InputArgs *input_args);

#endif /* SEIMS_MPI_H */
