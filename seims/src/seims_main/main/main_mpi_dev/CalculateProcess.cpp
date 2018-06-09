#include "CalculateProcess.h"

#include <map>
#include <set>
#include <vector>

#include "utils_time.h"
#include "DataCenterMongoDB.h"
#include "invoke.h"
#include "ModelMain.h"
#include "CombineRaster.h"
#include "text.h"

#include "parallel.h"
#include "ManagementProcess.h"

using namespace utils_time;
using namespace utils_array;
using std::map;
using std::set;
using std::vector;

void BuildSubbasinLayers(const int rank, int size, const int max_task_len,
                         const int n_subbasins, const bool include_channel,
                         int*& p_group_id,
                         int& max_lyr_id, int*& p_lyr_id_all,
                         int*& p_subbasin_ids_all, int*& p_upstream_nums_all,
                         map<int, int>& subbasin_rank,
                         map<int, vector<int> >& src_subbsn_layers,
                         map<int, vector<int> >& non_src_subbsn_layers,
                         set<int>& down_stream_set,
                         set<int>& down_stream_id_set) {

    for (int i = 0; i < size * max_task_len; i++) {
        if (p_subbasin_ids_all[i] < 0) continue;
        subbasin_rank[p_subbasin_ids_all[i]] = p_group_id[i] / max_task_len;
    }
#ifdef _DEBUG
    cout << "Subbasin ID -> Rank ID" << endl;
    for (auto it = subbasin_rank.begin(); it != subbasin_rank.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
#endif
    if (include_channel) {
        for (int i = 0; i < n_subbasins; i++) {
            int subbsn_idx = rank * max_task_len + i;
            // classification according to the Layering method, i.e., up-down and down-up orders.
            int stream_order = p_lyr_id_all[subbsn_idx];
            if (stream_order > max_lyr_id) { max_lyr_id = stream_order; }
            if (stream_order > 1) {
                // all possible downstream subbasins (i.e., layer >= 2)
                down_stream_set.insert(i);                        // index of the array of current rank
                down_stream_id_set.insert(p_subbasin_ids_all[i]); // Subbasin ID
            }
            if (p_upstream_nums_all[subbsn_idx] == 0) {
                // source subbasins of each layer
                if (src_subbsn_layers.find(stream_order) == src_subbsn_layers.end()) {
                    src_subbsn_layers.insert(make_pair(stream_order, vector<int>()));
                }
                src_subbsn_layers[stream_order].push_back(i);
            } else {
                if (non_src_subbsn_layers.find(stream_order) == non_src_subbsn_layers.end()) {
                    non_src_subbsn_layers.insert(make_pair(stream_order, vector<int>()));
                }
                non_src_subbsn_layers[stream_order].push_back(i);
            }
        }
    } else {
        // No channel processes are simulated, all subbasins will be regarded as source subbasins
        src_subbsn_layers.insert(make_pair(1, vector<int>()));
        for (int i = 0; i < n_subbasins; i++) {
            src_subbsn_layers[1].push_back(i);
        }
    }
#ifdef _DEBUG
    cout << "Rank: " << rank << ", Source subbasins: ";
    for (auto it = src_subbsn_layers.begin(); it != src_subbsn_layers.end(); ++it) {
        cout << "Layer ID: " << it->first;
        for (auto idx_it = it->second.begin(); idx_it != it->second.end(); ++idx_it) {
            cout << p_subbasin_ids_all[rank * max_task_len + *idx_it] << ", ";
        }
        cout << endl;
    }
    cout << endl;
    cout << "Rank: " << rank << ", Non-Source subbasins: ";
    for (auto it = non_src_subbsn_layers.begin(); it != non_src_subbsn_layers.end(); ++it) {
        cout << "Layer ID: " << it->first;
        for (auto idx_it = it->second.begin(); idx_it != it->second.end(); ++idx_it) {
            cout << p_subbasin_ids_all[rank * max_task_len + *idx_it] << ", ";
        }
        cout << endl;
    }
    cout << endl;
#endif /* _DEBUG */
}

void CalculateProcess(InputArgs* input_args, const int rank, const int size) {
    StatusMessage(("Enter computing process, Rank: " + ValueToString(rank)).c_str());
    /******************* Load parallel task scheduing information *******************/
    double tstart = MPI_Wtime();
    /// Create task information arrays
    int max_task_len;                   /// Max. task number
    int* p_group_id = nullptr;          /// Group ID which is corresponding to the rank id
    int* p_subbasin_ids_all = nullptr;  /// Subbasin IDs in all groups
    int* p_lyr_id_all = nullptr;        /// Layering number of each subbasins
    int* p_downstream_all = nullptr;    /// Down stream subbasin ID of each subbasin
    int* p_upstream_nums_all = nullptr; /// Upstream subbasin numbers of each subbasin
    int* p_upstream_ids_all = nullptr;  /// Upstream subbasin IDs of each subbasin

    if (rank == MASTER_RANK) {
        ManagementProcess(input_args, size, max_task_len, p_group_id, p_subbasin_ids_all,
                          p_lyr_id_all, p_downstream_all, p_upstream_nums_all, p_upstream_ids_all);
    }
    MPI_Barrier(MCW); /// Wait for master rank

    MPI_Bcast(&max_task_len, 1, MPI_INT, MASTER_RANK, MCW);
    int n_task_all = max_task_len * size;
#ifdef _DEBUG
    cout << "max. task length: " << max_task_len << endl;
#endif /* _DEBUG */
    /// Initialize arrays for other ranks
    if (rank != MASTER_RANK) {
        Initialize1DArray(size, p_group_id, -1);
        Initialize1DArray(n_task_all, p_subbasin_ids_all, -1);
        Initialize1DArray(n_task_all, p_lyr_id_all, -1);
        Initialize1DArray(n_task_all, p_downstream_all, -1);
        Initialize1DArray(n_task_all, p_upstream_nums_all, 0);
        Initialize1DArray(n_task_all * MAX_UPSTREAM, p_upstream_ids_all, -1);
    }
    MPI_Barrier(MCW); /// Wait for non-master rank

    MPI_Bcast(p_group_id, size, MPI_INT, MASTER_RANK, MCW);
    MPI_Bcast(p_subbasin_ids_all, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Bcast(p_lyr_id_all, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Bcast(p_downstream_all, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Bcast(p_upstream_nums_all, n_task_all, MPI_INT, MASTER_RANK, MCW);
    MPI_Bcast(p_upstream_ids_all, n_task_all * MAX_UPSTREAM, MPI_INT, MASTER_RANK, MCW);
    if (rank == MASTER_RANK) {
        StatusMessage("Tasks are dispatched.");
    }
#ifdef _DEBUG
    for (int i = 0; i < size; i++) {
        cout << "group id, subbasin ids, layer numbers, downstream ids, upstream numbers, upstream ids" << endl;
        cout << p_group_id[i] << ", [";
        for (int j = 0; j < max_task_len; j++) {
            if (p_subbasin_ids_all[i * max_task_len + j] < 0) break;
            cout << p_subbasin_ids_all[i * max_task_len + j] << ", ";
        }
        cout << "], [";
        for (int j = 0; j < max_task_len; j++) {
            if (p_lyr_id_all[i * max_task_len + j] < 0) break;
            cout << p_lyr_id_all[i * max_task_len + j] << ", ";
        }
        cout << "], [";
        for (int j = 0; j < max_task_len; j++) {
            if (p_downstream_all[i * max_task_len + j] < 0) break;
            cout << p_downstream_all[i * max_task_len + j] << ", ";
        }
        cout << "], [";
        for (int j = 0; j < max_task_len; j++) {
            if (p_upstream_nums_all[i * max_task_len + j] < 0) break;
            cout << p_upstream_nums_all[i * max_task_len + j] << ", ";
        }
        cout << "], [";
        for (int j = 0; j < max_task_len; j++) {
            cout << "[";
            for (int k = 0; k < MAX_UPSTREAM; k++) {
                if (p_upstream_ids_all[i * max_task_len * MAX_UPSTREAM + j * MAX_UPSTREAM + k] < 0) break;
                cout << p_upstream_ids_all[i * max_task_len * MAX_UPSTREAM + j * MAX_UPSTREAM + k] << ", ";
            }
            cout << "], ";
        }
        cout << "]" << endl;
    }
#endif /* _DEBUG */
    double load_task_tmp = MPI_Wtime() - tstart; /// Time of load parallel task sceduling information.
    double load_task_t;
    MPI_Reduce(&load_task_tmp, &load_task_t, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);

    /******************* Actual calculation *******************/
    int n_subbasins = 0; // subbasin number of current rank
    for (int i = 0; i < max_task_len; i++) {
        if (p_subbasin_ids_all[rank * max_task_len + i] > 0) n_subbasins++;
    }
    if (n_subbasins == 0) {
        cout << "No task for rank " << rank << endl;
        MPI_Abort(MCW, 1);
    }

    tstart = MPI_Wtime(); /// Start to construct objects of subbasin models
    /// Get module path, i.e., the path of executable and dynamic libraries
    string module_path = GetAppPath();
    /// Connect to MongoDB
    MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
    if (nullptr == mongo_client) {
        throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
    }
    /// Create module factory
    ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args);
    if (nullptr == module_factory) {
        throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
    }
    /// Get dynamic parameter count which need to be transferred across upstream-downstream subbasins
    int tranfer_count = module_factory->GetTransferredInputsCount();
    /// Create lists of data center objects and SEIMS model objects
    vector<DataCenterMongoDB *> data_center_list;
    vector<ModelMain *> model_list;
    data_center_list.reserve(n_subbasins);
    model_list.reserve(n_subbasins);
    for (int i = 0; i < n_subbasins; i++) {
        /// Create data center according to subbasin number
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client,
                                                               module_factory,
                                                               p_subbasin_ids_all[rank * max_task_len + i]);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model = new ModelMain(data_center, module_factory);

        data_center_list.push_back(data_center);
        model_list.push_back(model);
    }
    StatusMessage(("Rank " + ValueToString(rank) + " construct models done!").c_str());
    /// Reduce for model constructing time, which also input time
    double model_construct_tmp = MPI_Wtime() - tstart;
    double model_construct_t;
    MPI_Reduce(&model_construct_tmp, &model_construct_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
    double input_t = load_task_t + model_construct_t;
    if (rank == MASTER_RANK) {
        cout << "[TIMESPAN][IO]\tINPUT\t" << std::fixed << setprecision(3) << input_t << endl;
    }

    tstart = MPI_Wtime(); /// Start simulation
    /*! Subbasin object -> rank ID (i.e., group ID)
     * Key: Subbasin ID
     * Value: rank ID
     */
    map<int, int> subbasin_rank;
    int max_lyr_id = 1; /// Maximum layering ID
    /*! Source subbasins in each layer in current slave rank
     * Key: Layering ID
     * Value: Source subbasin indexes in current rank
     */
    map<int, vector<int> > src_subbsn_layers;
    /*! Non source subbasins in each layer in current slave rank
     * Key: Layering ID
     * Value: Non source subbasin indexes in current rank
     */
    map<int, vector<int> > non_src_subbsn_layers;
    // Used to find if the downstream subbasin of a finished subbsin is in the same process,
    //   if so, the MPI send operation is not necessary.
    //   the `set` container is more efficient for the 'find' operation
    set<int> down_stream_set;    // Indexes of not-first layer subbasins
    set<int> down_stream_id_set; // Subbasin IDs of not-first layer subbasins
    bool include_channel = model_list[0]->IncludeChannelProcesses();
    BuildSubbasinLayers(rank, size, max_task_len, n_subbasins, include_channel,
                        p_group_id, max_lyr_id, p_lyr_id_all,
                        p_subbasin_ids_all, p_upstream_nums_all,
                        subbasin_rank,
                        src_subbsn_layers, non_src_subbsn_layers,
                        down_stream_set, down_stream_id_set);

    //double t_slope = 0.0;   /// Time of hillslope processes
    //double t_channel = 0.0; /// Time of channel routing processes
    /// Create buffer for passing values across subbasins
    float* buf = nullptr;
    int buflen = MSG_LEN + tranfer_count;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    /// Initialize the transferred values of subbasins in current process,
    ///   NO NEED to create and release in each timestep.
    map<int, float *> transfer_values_map; // key is subbasinID, value is transferred values
    for (int i = 0; i < n_subbasins; i++) {
        float* tfvalues = nullptr;
        Initialize1DArray(tranfer_count, tfvalues, NODATA_VALUE);
        transfer_values_map.insert(make_pair(p_subbasin_ids_all[rank * max_task_len + i], tfvalues));
    }

    /// Get timesteps
    DataCenterMongoDB* dc = data_center_list[0];
    time_t dt_hs = dc->GetSettingInput()->getDtHillslope();
    time_t dt_ch = dc->GetSettingInput()->getDtChannel();
    time_t cur_time = dc->GetSettingInput()->getStartTime();
    int start_year = GetYear(cur_time);
    int n_hs = CVT_INT(dt_ch / dt_hs);

    MPI_Request request;
    MPI_Status status;

    // Simulation loop
    int sim_loop_num = 0; /// Simulation loop number according to channel routing time-step
    for (; cur_time <= dc->GetSettingInput()->getEndTime(); cur_time += dt_ch) {
        sim_loop_num += 1;
        if (rank == MASTER_RANK) StatusMessage(ConvertToString2(&cur_time).c_str());
        int year_idx = GetYear(cur_time) - start_year;
        //set<int> done_i_ds; // save subbasin IDs that have been executed
        // Execute by layering orders
        for (int ilyr = 1; ilyr <= max_lyr_id; ilyr++) {
#ifdef _DEBUG
            cout << "rank: " << rank << ", Layer " << ilyr << endl;
#endif
            // 1. Execute subbasins that does not depend on others
            if (src_subbsn_layers.find(ilyr) != src_subbsn_layers.end()) {
                for (auto it_src = src_subbsn_layers[ilyr].begin(); it_src != src_subbsn_layers[ilyr].end(); ++it_src) {
                    int subbasin_id = p_subbasin_ids_all[rank * max_task_len + *it_src];
#ifdef _DEBUG
                    cout << "    source subbasin: " << subbasin_id << endl;
#endif
                    // 1.1 Execute hillslope and channel processes, just like the serial version
                    ModelMain* psubbasin = model_list[*it_src];
                    for (int i = 0; i < n_hs; i++) {
                        psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                    }
                    psubbasin->StepChannel(cur_time, year_idx);
                    psubbasin->AppendOutputData(cur_time);

                    if (!include_channel) { continue; }
                    // 1.2 If the downstream subbasin is in this process,
                    //     there is no need to transfer values to the master process
                    if (subbasin_rank[subbasin_id] == rank) {
                        psubbasin->GetTransferredValue(transfer_values_map[subbasin_id]);
                        //done_i_ds.insert(subbasin_id);
                        continue;
                    }
                    // 1.3 Otherwise, the transferred values of current subbasin should be sent to another rank
                    psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                    cout << "Rank: " << rank << ", send subbasinID: " << subbasin_id << " tfValues: ";
                    for (int itf = MSG_LEN; itf < buflen; itf++) {
                        cout << buf[itf] << ", ";
                    }
                    cout << endl;
#endif
                    int dest_rank = subbasin_rank[p_downstream_all[rank * max_task_len + *it_src]];
                    buf[0] = 1.f;                   // message type: Send subbasin data to another rank
                    buf[1] = CVT_FLT(subbasin_id);  // subbasin ID
                    buf[2] = CVT_FLT(sim_loop_num); // simulation loop number
                    MPI_Isend(buf, buflen, MPI_FLOAT, dest_rank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                } /* src_subbsn_layers[ilyr] loop */
            }     /* If src_subbsn_layers has ilyr */

            // 2. Execute subbasins that depend on others
            if (non_src_subbsn_layers.find(ilyr) == non_src_subbsn_layers.end()) { continue; }
            // some layers may absent in current slave rank
            if (!include_channel) { continue; }

            // 2.1 Execute hillslope processes
            set<int> todo_set; // To be executed subbasin indexes of current layer
            for (auto it_down = non_src_subbsn_layers[ilyr].begin();
                 it_down != non_src_subbsn_layers[ilyr].end(); ++it_down) {
                int subbasin_id = p_subbasin_ids_all[rank * max_task_len + *it_down];
#ifdef _DEBUG
                cout << "    non-source subbasin(hillslope): " << subbasin_id << endl;
#endif
                ModelMain* psubbasin = model_list[*it_down];
                for (int i = 0; i < n_hs; i++) {
                    psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                }
                todo_set.insert(*it_down);
            }

            // 2.2 The channel routing of downStream subbasin will be calculated
            //     if its upstream subbasins are already calculated
            set<int> cando_set;
            while (!todo_set.empty()) {
                // 2.2.1 Find all subbasins that the channel routing can be done
                //       without asking the master process for data
                for (auto it_todo = todo_set.begin(); it_todo != todo_set.end();) {
                    bool up_finished = true;
                    int cur_subbsn_idx = rank * max_task_len + *it_todo;
                    for (int j = 0; j < p_upstream_nums_all[cur_subbsn_idx]; j++) {
                        int up_idx = rank * max_task_len * MAX_UPSTREAM + *it_todo * MAX_UPSTREAM + j;
                        int up_id = p_upstream_ids_all[up_idx]; /// subbasin ID
                        int up_rank = subbasin_rank[up_id];     /// the corresponding rank ID
                        MPI_Irecv(transfer_values_map[up_id], buflen, MPI_FLOAT, up_rank, WORK_TAG, MCW, &request);
                        MPI_Wait(&request, &status);
                        //// if can not find upstreams, this subbasin can not be done
                        //if (done_i_ds.find(up_id) == done_i_ds.end()) {
                        //    up_finished = false;
                        //    break;
                        //}
                    }
                    if (up_finished) {
                        cando_set.insert(*it_todo);
                        todo_set.erase(it_todo++);
                    } else {
                        ++it_todo;
                    }
                }
#ifdef _DEBUG
                cout << "rank " << rank << ", todo set (subbasinID): ";
                for (auto it_todo = todo_set.begin(); it_todo != todo_set.end(); ++it_todo) {
                    cout << p_subbasin_ids_all[rank * max_task_len + *it_todo] << ", ";
                }
                cout << ", cando set (subbasinID): ";
                for (auto it_cando = cando_set.begin(); it_cando != cando_set.end(); ++it_cando) {
                    cout << p_subbasin_ids_all[rank * max_task_len + *it_cando] << ", ";
                }
                cout << endl;
#endif
                // 2.2.2 If can not find subbasins to calculate according to local information,
                //       ask the master process if there are new upstream subbasins calculated
                if (!cando_set.empty()) {
                    //                    buf[0] = 2.f; // Ask for the already calculated subbasins from other slave ranks
                    //                    buf[1] = CVT_FLT(group_id);
                    //                    buf[2] = CVT_FLT(rank);
                    //                    buf[3] = CVT_FLT(sim_loop_num); // simulation loop number
                    //
                    //                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    //                    MPI_Wait(&request, &status);
                    //                    // Wait and receive data from master rank
                    //                    int msg_len;
                    //                    MPI_Irecv(&msg_len, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    //                    MPI_Wait(&request, &status);
                    //
                    //                    float* pdata = new float[msg_len];
                    //                    MPI_Irecv(pdata, msg_len, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    //                    MPI_Wait(&request, &status);
                    //#ifdef _DEBUG
                    //                    cout << "recv count of subbasin data: " << msg_len / (tranfer_count + 1) << ", subbasinIDs: ";
                    //                    for (int ri = 0; ri < msg_len / (tranfer_count + 1); ri++) {
                    //                        cout << CVT_INT(pdata[ri * (tranfer_count + 1)]) << ", ";
                    //                    }
                    //                    cout << endl;
                    //#endif
                    //                    for (int ri = 0; ri < msg_len / (tranfer_count + 1); ri++) {
                    //                        int recv_subid = CVT_INT(pdata[ri * (tranfer_count + 1)]);
                    //                        if (transfer_values_map.find(recv_subid) == transfer_values_map.end()) {
                    //                            float* tfvalues = nullptr;
                    //                            Initialize1DArray(tranfer_count, tfvalues, NODATA_VALUE);
                    //                            transfer_values_map.insert(make_pair(recv_subid, tfvalues));
                    //                        }
                    //                        for (int vi = 0; vi < tranfer_count; vi++) {
                    //                            transfer_values_map[recv_subid][vi] = pdata[ri * (tranfer_count + 1) + vi + 1];
                    //                        }
                    //                        done_i_ds.insert(recv_subid);
                    //                    }
                    //                    Release1DArray(pdata);
                    //                } else {
                    // 2.2.3 The required subbasin data has been satisfied,
                    //       execute subbasin in cando_set, and send data to master rank if necessary
                    for (auto it_cando = cando_set.begin(); it_cando != cando_set.end();) {
                        int cur_subbsn_idx = rank * max_task_len + *it_cando;
                        int cur_subbsin_id = p_subbasin_ids_all[cur_subbsn_idx];
#ifdef _DEBUG
                        cout << "    non-source subbasin(channel): " << p_subbasin_ids_all[cur_subbsn_idx] << endl;
#endif
                        ModelMain* psubbasin = model_list[*it_cando];
                        for (int j = 0; j < p_upstream_nums_all[cur_subbsn_idx]; j++) {
                            int subbsn_idx = rank * max_task_len * MAX_UPSTREAM + *it_cando * MAX_UPSTREAM + j;
                            int subbsn_id = p_upstream_ids_all[subbsn_idx]; /// subbasin ID
                            psubbasin->SetTransferredValue(subbsn_id, transfer_values_map[subbsn_id]);
                        }
                        psubbasin->StepChannel(cur_time, year_idx);
                        psubbasin->AppendOutputData(cur_time);

                        if (subbasin_rank[p_downstream_all[cur_subbsn_idx]] == rank) {
                            psubbasin->GetTransferredValue(transfer_values_map[cur_subbsin_id]);
                            //done_i_ds.insert(cur_subbsin_id);
                            cando_set.erase(it_cando++); // remove current subbasin from cando_set
                            continue;
                        }
                        // Else, transfer the result to another rank
                        buf[0] = 1.f;                         // message type: Send subbasin data to master rank
                        buf[1] = CVT_FLT(cur_subbsin_id); // subbasin ID
                        buf[2] = CVT_FLT(sim_loop_num); // simulation loop number
                        psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                        cout << "rank: " << rank << ", send subbasinID: " << cur_subbsin_id << ", tfValues: ";
                        for (int itf = MSG_LEN; itf < buflen; itf++) {
                            cout << buf[itf] << ", ";
                        }
                        cout << endl;
#endif
                        int dest_rank = subbasin_rank[cur_subbsin_id];
                        MPI_Isend(buf, buflen, MPI_FLOAT, dest_rank, WORK_TAG, MCW, &request);
                        MPI_Wait(&request, &status);
                        cando_set.erase(it_cando++);
                    } /* cando_set loop of current layer */
                }     /* cando_set is not empty */
            }         /* while todo_set is not empty */
        } /* subbasin layers loop */
        MPI_Barrier(MCW);
    } /* timestep loop */

    double comp_tmp = MPI_Wtime() - tstart;
    double comp_t;
    MPI_Reduce(&comp_tmp, &comp_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
    if (rank == MASTER_RANK) {
        cout << "[TIMESPAN][COMPUTING]\tALL\t" << std::fixed << setprecision(3) << comp_t << endl;
    }
    /***************  Outputs and combination  ***************/
    tstart = MPI_Wtime();
    for (int i = 0; i < n_subbasins; i++) {
        model_list[i]->Output();
    }
    double output_tmp = MPI_Wtime() - tstart;
    double output_t;
    MPI_Reduce(&output_tmp, &output_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);

    /*** Combine raster outputs serially by one processor. ***/
    /// The operation could be considered as post-process,
    ///   therefore, the time-consuming is not included.
    if (rank == MASTER_RANK) {
        MongoGridFs* gfs = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput* outputs = data_center_list[0]->GetSettingOutput();
        for (auto it = outputs->m_printInfos.begin(); it != outputs->m_printInfos.end(); ++it) {
            for (auto item_it = (*it)->m_PrintItems.begin(); item_it != (*it)->m_PrintItems.end(); ++item_it) {
                PrintInfoItem* item = *item_it;
                if (item->m_nLayers >= 1) {
                    // Only need to handle raster data
                    StatusMessage(("Combining raster: " + item->Corename).c_str());
                    CombineRasterResultsMongo(gfs, item->Corename, data_center_list[0]->GetSubbasinsCount(),
                                              data_center_list[0]->GetOutputScenePath());
                }
            }
        }
        // clean up
        delete gfs;
    }
    /*** End of Combine raster outputs. ***/
    if (rank == MASTER_RANK) {
        cout << "[TIMESPAN][IO]\tOUTPUT\t" << std::fixed << setprecision(3) << output_t << endl;
        double io_t = load_task_t + input_t + output_t;
        cout << "[TIMESPAN][IO]\tALL\t" << std::fixed << setprecision(3) << io_t << endl;
        double all_t = io_t + comp_t;
        cout << "[TIMESPAN][CALCULATION]\tALL\t" << std::fixed << setprecision(3) << all_t << endl;
    }
    MPI_Barrier(MCW);

    // clean up
    for (auto it = transfer_values_map.begin(); it != transfer_values_map.end();) {
        if (it->second != nullptr) {
            Release1DArray(it->second);
            it->second = nullptr;
        }
        transfer_values_map.erase(it++);
    }
    for (auto it = model_list.begin(); it != model_list.end();) {
        if (*it != nullptr) {
            delete *it;
        }
        it = model_list.erase(it);
    }
    for (auto it = data_center_list.begin(); it != data_center_list.end();) {
        if (*it != nullptr) {
            delete *it;
        }
        it = data_center_list.erase(it);
    }
    delete module_factory;
    delete mongo_client;

    Release1DArray(buf);
    Release1DArray(p_subbasin_ids_all);
    Release1DArray(p_lyr_id_all);
    Release1DArray(p_downstream_all);
    Release1DArray(p_upstream_nums_all);
    Release1DArray(p_upstream_ids_all);
}
