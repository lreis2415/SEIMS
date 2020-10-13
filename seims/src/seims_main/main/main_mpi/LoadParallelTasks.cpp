#include "LoadParallelTasks.h"

#include "ReadReachTopology.h"
#include "parallel.h"
#include "Logging.h"

int ManagementProcess(MongoClient* mclient, InputArgs* input_args, const int size, TaskInfo* task) {
    /// 1. Read river topology data, abort(1) if failed.
    map<int, SubbasinStruct *> subbasin_map;
    set<int> group_set;
    if (CreateReachTopology(mclient, input_args->model_name, input_args->grp_mtd,
                            size, subbasin_map, group_set) != 0) {
        LOG(TRACE) << "Read and create reaches topology information failed.";
        MPI_Abort(MCW, 1);
    }
    if (size_t(size) != group_set.size()) {
        group_set.clear();
        LOG(TRACE) << "The number of slave processes (" << size << ") is not consist with the group number("
        << group_set.size() << ").";
        MPI_Abort(MCW, 1);
    }
    /// 2. Scatter the group set (i.e., parallel tasks) to all processes

    /// 2.1 Get the subbasin id list of different groups
    /* key: Group ID
     * value: Subbasin IDs
     */
    map<int, vector<int> > group_map;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
#ifdef HAS_VARIADIC_TEMPLATES
        group_map.emplace(*it, vector<int>());
#else
        group_map.insert(make_pair(*it, vector<int>()));
#endif
    }
    // get the subbasin ID list of different groups
    for (auto it = subbasin_map.begin(); it != subbasin_map.end(); ++it) {
        group_map[it->second->group].emplace_back(it->second->id);
    }
    // get the maximum length of the task assignment message
    task->max_len = 0;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        if (CVT_INT(group_map[*it].size()) > task->max_len) {
            task->max_len = CVT_INT(group_map[*it].size());
        }
    }

    CLOG(TRACE, LOG_INIT) << "Group set: ";
    for (auto it = group_map.begin(); it != group_map.end(); ++it) {
        std::ostringstream oss;
        oss << "  group id: " << it->first << ", subbasin IDs: ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            oss << *it2 << ", ";
        }
        CLOG(TRACE, LOG_INIT) << oss.str();
    }
    CLOG(TRACE, LOG_INIT) << "  max task length: " << task->max_len;

    int n_task_all = task->max_len * size;
    // initialization
    Initialize1DArray(n_task_all, task->subbsn_id, -1);
    Initialize1DArray(n_task_all, task->lyr_id, -1);
    Initialize1DArray(n_task_all, task->down_id, -1);
    Initialize1DArray(n_task_all, task->up_count, 0);
    Initialize1DArray(n_task_all * MAX_UPSTREAM, task->up_ids, -1);

    int igroup = 0;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        int group_index = igroup * task->max_len;
        for (size_t i = 0; i < group_map[*it].size(); i++) {
            int id = group_map[*it][i];
            task->subbsn_id[group_index + i] = id;
            task->lyr_id[group_index + i] = input_args->lyr_mtd == UP_DOWN
                                                ? subbasin_map[id]->updown_order
                                                : subbasin_map[id]->downup_order;
            if (subbasin_map[id]->down_stream != nullptr) {
                task->down_id[group_index + i] = subbasin_map[id]->down_stream->id;
            }
            int n_ups = CVT_INT(subbasin_map[id]->up_streams.size());
            task->up_count[group_index + i] = n_ups;
            if (n_ups > MAX_UPSTREAM) {
                CLOG(TRACE, LOG_INIT) << "The number of upstreams exceeds MAX_UPSTREAM: " << MAX_UPSTREAM;
                MPI_Abort(MCW, 1);
            }
            for (int j = 0; j < n_ups; j++) {
                task->up_ids[MAX_UPSTREAM * (group_index + i) + j] = subbasin_map[id]->up_streams[j]->id;
            }
        }
        igroup++;
    }
    // send the information to all processes
    CLOG(TRACE, LOG_INIT) << "Sending tasks to the all processes...";

    CLOG(TRACE, LOG_INIT) << "  pTaskSubbasinID, pGroupID, pLayerNumber, pDownStream, pUpNums";

    for (int i = 0; i < n_task_all; i++) {
        if (task->subbsn_id[i] < 0) continue;
        CLOG(TRACE, LOG_INIT) << "  " << task->subbsn_id[i] << ", " << i / task->max_len << ", " << task->lyr_id[i] << ", "
        << task->down_id[i] << ", " << task->up_count[i];
    }

    return 0;
}

int LoadTasks(MongoClient* client, InputArgs* input_args, const int size, const int rank, TaskInfo* task) {
    CLOG(TRACE, LOG_INIT) << "Loading parallel task scheduing information...";
    if (rank == MASTER_RANK) {
        ManagementProcess(client, input_args, size, task);
    }
    MPI_Barrier(MCW); /// Wait for master rank
    MPI_Bcast(&task->max_len, 1, MPI_INT, MASTER_RANK, MCW);
    int n_task_all = task->max_len * size;

    CLOG(TRACE, LOG_INIT) << "Max. task length: " << task->max_len << endl;

    /// Initialize arrays for other ranks
    if (rank != MASTER_RANK) {
        Initialize1DArray(n_task_all, task->subbsn_id, -1);
        Initialize1DArray(n_task_all, task->lyr_id, -1);
        Initialize1DArray(n_task_all, task->down_id, -1);
        Initialize1DArray(n_task_all, task->up_count, 0);
        Initialize1DArray(n_task_all * MAX_UPSTREAM, task->up_ids, -1);
    }
    MPI_Barrier(MCW); /// Wait for non-master rank

    MPI_Bcast(task->subbsn_id, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Barrier(MCW);
    MPI_Bcast(task->lyr_id, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Barrier(MCW);
    MPI_Bcast(task->down_id, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Barrier(MCW);
    MPI_Bcast(task->up_count, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Barrier(MCW);
    MPI_Bcast(task->up_ids, n_task_all * MAX_UPSTREAM, MPI_INT, MASTER_RANK, MCW);
    MPI_Barrier(MCW);
    if (rank == MASTER_RANK) {
        CLOG(TRACE, LOG_INIT) << "Tasks are dispatched.";
    }

    for (int i = 0; i < size; i++) {
        CLOG(TRACE, LOG_INIT) << "group id, subbasin ids, layer numbers, downstream ids, upstream numbers, upstream ids";
        std::ostringstream oss;
        oss << i << ", [";
        for (int j = 0; j < task->max_len; j++) {
            if (task->subbsn_id[i * task->max_len + j] < 0) break;
            oss << task->subbsn_id[i * task->max_len + j] << ", ";
        }
        oss << "], [";
        for (int j = 0; j < task->max_len; j++) {
            if (task->lyr_id[i * task->max_len + j] < 0) break;
            oss << task->lyr_id[i * task->max_len + j] << ", ";
        }
        oss << "], [";
        for (int j = 0; j < task->max_len; j++) {
            if (task->down_id[i * task->max_len + j] < 0) break;
            oss << task->down_id[i * task->max_len + j] << ", ";
        }
        oss << "], [";
        for (int j = 0; j < task->max_len; j++) {
            if (task->up_count[i * task->max_len + j] < 0) break;
            oss << task->up_count[i * task->max_len + j] << ", ";
        }
        oss << "], [";
        for (int j = 0; j < task->max_len; j++) {
            oss << "[";
            for (int k = 0; k < MAX_UPSTREAM; k++) {
                if (task->up_ids[i * task->max_len * MAX_UPSTREAM + j * MAX_UPSTREAM + k] < 0) break;
                oss << task->up_ids[i * task->max_len * MAX_UPSTREAM + j * MAX_UPSTREAM + k] << ", ";
            }
            oss << "], ";
        }
        oss << "]";
        CLOG(TRACE, LOG_INIT) << oss.str();
    }
    return 0;
}
