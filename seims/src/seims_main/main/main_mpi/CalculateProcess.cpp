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
#include "TaskInformation.h"
#include "LoadParallTasks.h"

using namespace utils_time;
using namespace utils_array;
using std::map;
using std::set;
using std::vector;

void CalculateProcess(InputArgs* input_args, const int rank, const int size) {
    StatusMessage(("Computing process, Rank: " + ValueToString(rank)).c_str());
    double tstart = MPI_Wtime();
    /// Get module path, i.e., the path of executable and dynamic libraries
    string module_path = GetAppPath();
    /// Connect to MongoDB
    MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
    if (nullptr == mongo_client) {
        throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
    }
    /// Load parallel task scheduling information and record time-consuming.
    TaskInfo* task_info = new TaskInfo(size, rank);
    LoadTasks(mongo_client, input_args, size, rank, task_info);
    if (!task_info->Build()) {
        cout << "Rank: " << rank << ", task information build failed!" << endl;
        MPI_Abort(MCW, 1);
    }

    double load_task_tmp = MPI_Wtime() - tstart;
    double load_task_t;
    MPI_Reduce(&load_task_tmp, &load_task_t, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);

    /******************* Actual calculation *******************/
    tstart = MPI_Wtime();                             /// Start to construct objects of subbasin models
    int n_subbasins = task_info->GetSubbasinNumber(); // subbasin number of current rank
    if (n_subbasins == 0) {
        cout << "No task for rank " << rank << endl;
        MPI_Abort(MCW, 1);
    }
    int max_lyr_id_all = task_info->GetGlobalMaxLayerID(); /// Global maximum layering ID

    /// Create module factory
    ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args);
    if (nullptr == module_factory) {
        throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
    }
    /// Get dynamic parameter count which need to be transferred across upstream-downstream subbasins
    int transfer_count = module_factory->GetTransferredInputsCount();
    /// Create lists of data center objects and SEIMS model objects
    map<int, DataCenterMongoDB *> data_center_map;
    map<int, ModelMain *> model_map;
    vector<int>& rank_subbsn_ids = task_info->GetRankSubbasinIDs();
    for (auto it_id = rank_subbsn_ids.begin(); it_id != rank_subbsn_ids.end(); ++it_id) {
        /// Create data center according to subbasin number
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client,
                                                               module_factory, *it_id);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model = new ModelMain(data_center, module_factory);

        data_center_map.insert(make_pair(*it_id, data_center));
        model_map.insert(make_pair(*it_id, model));
    }

    /// Get some variables
    bool include_channel = model_map.begin()->second->IncludeChannelProcesses();
    time_t dt_hs = data_center_map.begin()->second->GetSettingInput()->getDtHillslope();
    time_t dt_ch = data_center_map.begin()->second->GetSettingInput()->getDtChannel();
    time_t start_time = data_center_map.begin()->second->GetSettingInput()->getStartTime();
    time_t end_time = data_center_map.begin()->second->GetSettingInput()->getEndTime();
    int start_year = GetYear(start_time);
    int n_hs = CVT_INT(dt_ch / dt_hs);

    // Get task related variables
    map<int, int>& subbasin_rank = task_info->GetSubbasinRank();
    map<int, int>& downstream = task_info->GetDownstreamID();
    map<int, vector<int> >& upstreams = task_info->GetUpstreamIDs();
    map<int, vector<int> >& subbsn_layers = task_info->GetLayerSubbasinIDs();

    /// Create buffer for passing values across subbasins
    float* buf = nullptr;
    int buflen = MSG_LEN + transfer_count;
    Initialize1DArray(buflen, buf, NODATA_VALUE);

    /// Initialize the transferred values of subbasins in current process and received from other processes
    ///   NO NEED to create and release in each timestep.
    /// Determining the maximum time slices
    int max_time_slices = CVT_INT(ceil(CVT_FLT(end_time - start_time) / CVT_FLT(dt_ch) / CVT_FLT(max_lyr_id_all)));
    int time_slice = input_args->time_slices; // input from arguments
    if (time_slice < 0 || time_slice > max_time_slices) time_slice = max_time_slices;
    if (time_slice < 2) time_slice = 2;
    int multiplier = CVT_INT(ceil(CVT_FLT(max_time_slices) / CVT_FLT(time_slice)));
    if (input_args->skd_mtd == SPATIAL) multiplier = 1;

    task_info->MallocTransferredValues(transfer_count, multiplier);
    /// Transferred values of subbasins in current rank with timestep stamp
    map<int, map<int, float *> >& ts_subbsn_tf_values = task_info->GetSubbasinTransferredValues();
    /// Flag to indicate the subbasin was executed or not
    map<int, map<int, bool> > ts_subbsn_flag;
    /// Received transferred values of subbasins in current rank with timestep stamp
    map<int, map<int, float *> >& recv_ts_subbsn_tf_values = task_info->GetReceivedSubbasinTransferredValues();

    StatusMessage(("Rank " + ValueToString(rank) + " construct models done!").c_str());
    /// Reduce for model constructing time, which also input time
    double model_construct_tmp = MPI_Wtime() - tstart;
    double model_construct_t;
    MPI_Reduce(&model_construct_tmp, &model_construct_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
    double input_t = load_task_t + model_construct_t;
    if (rank == MASTER_RANK) {
        cout << "[TIMESPAN][IO]\tINPUT\t" << std::fixed << setprecision(3) << input_t << endl;
    }

    MPI_Request request;
    MPI_Status status;

    // Simulation loop
    double t_slope = 0.;   /// Time of hillslope processes
    double t_channel = 0.; /// Time of channel routing processes
    double t_slope_start;
    double t_channel_start;
    int sim_loop_num = 0; /// Simulation loop number, which will be ciculated at 1 ~ max_lyr_id_all
    int exec_lyr_num = 1; // For SPATIAL scheduling method
    int max_loop_num = max_lyr_id_all * multiplier;
    tstart = MPI_Wtime(); /// Start simulation
    for (time_t ts = start_time; ts <= end_time; ts += dt_ch) {
        sim_loop_num += 1;
        if (rank == MASTER_RANK) {
            // cout << ConvertToString2(&ts) << endl;
        }
#ifdef _DEBUG
        cout << ConvertToString2(&ts) << ", sim_loop_num: " << sim_loop_num << endl;
#endif
        int year_idx = GetYear(ts) - start_year;
        // Execute by layering orders
        for (int ilyr = 1; ilyr <= max_lyr_id_all; ilyr++) {
            // if (subbsn_layers.find(ilyr) == subbsn_layers.end()) continue; // DO NOT UNCOMMENT THIS!
#ifdef _DEBUG
            cout << "Rank: " << rank << ", Layer " << ilyr << endl;
#endif
            if (input_args->skd_mtd == TEMPOROSPATIAL) exec_lyr_num = ilyr;

            for (int lyr_dlt = 0; lyr_dlt < exec_lyr_num; lyr_dlt++) {
                if (ts + dt_ch * lyr_dlt > end_time) break;
                int cur_ilyr = ilyr - lyr_dlt;
                int cur_sim_loop_num = sim_loop_num + lyr_dlt;
                // When cur_sim_loop_num exceeds max_lyr_id_all, recount it!
                if (cur_sim_loop_num > max_loop_num) cur_sim_loop_num %= max_loop_num;
                for (auto it = subbsn_layers[cur_ilyr].begin(); it != subbsn_layers[cur_ilyr].end(); ++it) {
                    int subbasin_id = *it;
                    time_t cur_time = ts + lyr_dlt * dt_ch;
                    // If subbasin_id already executed or the downstream subbasin has been executed, continue.
                    if (ts_subbsn_flag[cur_sim_loop_num][subbasin_id] ||
                        ts_subbsn_flag[cur_sim_loop_num][downstream[subbasin_id]]) {
                        continue;
                    }
                    // 1. Execute hillslope processes
                    t_slope_start = TimeCounting();
#ifdef _DEBUG
                    cout << "  " << ConvertToString2(&cur_time) <<
                            "  Hillslope process, subbasin: " << subbasin_id << endl;
#endif
                    ModelMain* psubbasin = model_map[*it];
                    for (int i = 0; i < n_hs; i++) {
                        psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                    }
                    t_slope += TimeCounting() - t_slope_start;
                    if (!include_channel) continue;

                    // 2. Execute channel processes
                    t_channel_start = TimeCounting();
#ifdef _DEBUG
                    cout << "  " << ConvertToString2(&cur_time) <<
                            "  Channel process, subbasin: " << subbasin_id << endl;
#endif

                    // 2.1 Set transferred data from upstreams
                    for (auto it_upid = upstreams[subbasin_id].begin();
                         it_upid != upstreams[subbasin_id].end(); ++it_upid) {
                        if (subbasin_rank[*it_upid] == rank) {
                            if (!ts_subbsn_flag[cur_sim_loop_num][*it_upid]) {
                                cout << "  " << ConvertToString2(&cur_time) <<
                                        ", failed when get data of subbasin " << *it_upid <<
                                        " of simulation loop: " << cur_sim_loop_num << endl;
                                MPI_Abort(MCW, 3);
                            }
                            psubbasin->SetTransferredValue(*it_upid, ts_subbsn_tf_values[cur_sim_loop_num][*it_upid]);
                            ts_subbsn_flag[cur_sim_loop_num][*it_upid] = false;
                        } else {
                            // receive data from the specific rank according to work_tag
                            int work_tag = *it_upid * 10000 + cur_sim_loop_num;;
                            MPI_Irecv(buf, buflen, MPI_FLOAT, subbasin_rank[*it_upid], work_tag, MCW, &request);
                            MPI_Wait(&request, &status);
#ifdef _DEBUG
                            cout << "Receive data of subbasin: " << *it_upid << " of sim_loop: " <<
                                    cur_sim_loop_num << " from rank: " << subbasin_rank[*it_upid] << ", tfValues: ";
                            for (int itf = MSG_LEN; itf < buflen; itf++) {
                                cout << buf[itf] << ", ";
                            }
                            cout << endl;
#endif
                            for (int vi = 0; vi < transfer_count; vi++) {
                                recv_ts_subbsn_tf_values[cur_sim_loop_num][*it_upid][vi] = buf[MSG_LEN + vi];
                            }
                            psubbasin->SetTransferredValue(*it_upid,
                                                           recv_ts_subbsn_tf_values[cur_sim_loop_num][*it_upid]);
                        }
                    }
                    psubbasin->StepChannel(cur_time, year_idx);
                    psubbasin->AppendOutputData(cur_time);

                    ts_subbsn_flag[cur_sim_loop_num][subbasin_id] = true;
                    // 2.2 If the downstream subbasin is in this process,
                    //     there is no need to transfer values to the master process
                    int downstream_id = downstream[subbasin_id];
                    if (downstream_id > 0 && subbasin_rank[downstream_id] == rank) {
                        psubbasin->GetTransferredValue(ts_subbsn_tf_values[cur_sim_loop_num][subbasin_id]);
                        t_channel += TimeCounting() - t_channel_start;
                        continue;
                    }
                    if (downstream_id < 0) {
                        // There is no need to get transferred values
                        t_channel += TimeCounting() - t_channel_start;
                        continue;
                    }
                    // 2.3 Otherwise, the transferred values of current subbasin should be sent to another rank
                    psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                    cout << "Rank: " << rank << ", send subbasinID: " << subbasin_id << " of sim_loop: " <<
                            cur_sim_loop_num << " -> Rank: " << subbasin_rank[downstream_id] << ", tfValues: ";
                    for (int itf = MSG_LEN; itf < buflen; itf++) {
                        cout << buf[itf] << ", ";
                    }
                    cout << endl;
#endif
                    int dest_rank = subbasin_rank[downstream[subbasin_id]];
                    buf[0] = CVT_FLT(subbasin_id);      // subbasin ID
                    buf[1] = CVT_FLT(cur_sim_loop_num); // simulation loop number
                    int work_tag = subbasin_id * 10000 + cur_sim_loop_num;
                    MPI_Isend(buf, buflen, MPI_FLOAT, dest_rank, work_tag, MCW, &request);
                    MPI_Wait(&request, &status);

                    t_channel += TimeCounting() - t_channel_start;
                } /* subbsn_layers[cur_ilyr] loop */
            }     /* loop of lyr_dlt = 0 to exec_lyr_num */
        }         /* If subbsn_layers has ilyr */

        // Reset flags of former one loop
        if (sim_loop_num > 1) {
            for (auto it_flag = ts_subbsn_flag[sim_loop_num - 1].begin();
                 it_flag != ts_subbsn_flag[sim_loop_num - 1].end(); ++it_flag) {
                it_flag->second = false;
            }
        }
        if (sim_loop_num % max_loop_num == 0) {
            // Reset flags of current loop
            for (auto it_flag = ts_subbsn_flag[sim_loop_num].begin();
                 it_flag != ts_subbsn_flag[sim_loop_num].end(); ++it_flag) {
                it_flag->second = false;
            }
            sim_loop_num = 0;
            MPI_Barrier(MCW);
        }
    } /* timestep loop */


    double slope_t;
    double channel_t;
    MPI_Reduce(&t_slope, &slope_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
    MPI_Reduce(&t_channel, &channel_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);

    double comp_tmp = MPI_Wtime() - tstart;
    double comp_t;
    MPI_Reduce(&comp_tmp, &comp_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
    if (rank == MASTER_RANK) {
        cout << "[TIMESPAN][COMPUTING]\tALL\t" << std::fixed << setprecision(3) << comp_t << endl;
        cout << "[TIMESPAN][COMPUTING]\tHillslope\t" << std::fixed << setprecision(3) << slope_t << endl;
        cout << "[TIMESPAN][COMPUTING]\tChannel\t" << std::fixed << setprecision(3) << channel_t << endl;
    }
#ifdef _DEBUG
    cout << "Rank: " << rank << endl;
    cout << "    [COMPUTING]\tALL\t" << std::fixed << setprecision(3) << comp_tmp << endl;
    cout << "    [COMPUTING]\tHillslope\t" << std::fixed << setprecision(3) << t_slope << endl;
    cout << "    [COMPUTING]\tChannel\t" << std::fixed << setprecision(3) << t_channel << endl;
    cout << "    [COMPUTING]\tBarrier\t" << std::fixed << setprecision(3) << comp_tmp - t_slope - t_channel << endl;
#endif
    /***************  Outputs and combination  ***************/
    tstart = MPI_Wtime();
    for (auto it_id = rank_subbsn_ids.begin(); it_id != rank_subbsn_ids.end(); ++it_id) {
        model_map[*it_id]->Output();
    }
    double output_tmp = MPI_Wtime() - tstart;
    double output_t;
    MPI_Reduce(&output_tmp, &output_t, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);

    /*** Combine raster outputs serially by one processor. ***/
    /// The operation could be considered as post-process,
    ///   therefore, the time-consuming is not included.
    if (rank == MASTER_RANK) {
        MongoGridFs* gfs = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput* outputs = data_center_map.begin()->second->GetSettingOutput();
        for (auto it = outputs->m_printInfos.begin(); it != outputs->m_printInfos.end(); ++it) {
            for (auto item_it = (*it)->m_PrintItems.begin(); item_it != (*it)->m_PrintItems.end(); ++item_it) {
                PrintInfoItem* item = *item_it;
                if (item->m_nLayers >= 1) {
                    // Only need to handle raster data
                    StatusMessage(("Combining raster: " + item->Corename).c_str());
                    CombineRasterResultsMongo(gfs, item->Corename,
                                              data_center_map.begin()->second->GetSubbasinsCount(),
                                              data_center_map.begin()->second->GetOutputScenePath());
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
    for (auto it = model_map.begin(); it != model_map.end();) {
        delete it->second;
        it->second = nullptr;
        model_map.erase(it++);
    }
    for (auto it = data_center_map.begin(); it != data_center_map.end();) {
        delete it->second;
        it->second = nullptr;
        data_center_map.erase(it++);
    }
    delete module_factory;
    delete mongo_client;
    delete task_info;
    if (buf != nullptr) Release1DArray(buf);
}
