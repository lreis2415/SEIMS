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
    int max_lyr_id = task_info->GetMaxLayerID(); /// Maximum layering ID
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
    bool include_channel = model_map.begin()->second->IncludeChannelProcesses();
    /// Get timesteps
    time_t dt_hs = data_center_map.begin()->second->GetSettingInput()->getDtHillslope();
    time_t dt_ch = data_center_map.begin()->second->GetSettingInput()->getDtChannel();
    time_t start_time = data_center_map.begin()->second->GetSettingInput()->getStartTime();
    time_t end_time = data_center_map.begin()->second->GetSettingInput()->getEndTime();
    int start_year = GetYear(start_time);
    int n_hs = CVT_INT(dt_ch / dt_hs);

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

    // Get task related variables
    map<int, int>& subbasin_rank = task_info->GetSubbasinRank();
    map<int, int>& downstream = task_info->GetDownstreamID();
    map<int, vector<int> >& upstreams = task_info->GetUpstreamIDs();
    map<int, vector<int> >& subbsn_layers = task_info->GetLayerSubbasinIDs();

    /// Create buffer for passing values across subbasins
    float* buf = nullptr;
    int buflen = MSG_LEN + transfer_count;
    Initialize1DArray(buflen, buf, NODATA_VALUE);

    /// Initialize the transferred values of subbasins in current process,
    ///   NO NEED to create and release in each timestep.
    map<int, float *> transfer_values_map; // key is subbasinID, value is transferred values
    for (auto it_id = rank_subbsn_ids.begin(); it_id != rank_subbsn_ids.end(); ++it_id) {
        float* tfvalues = nullptr;
        Initialize1DArray(transfer_count, tfvalues, NODATA_VALUE);
        transfer_values_map.insert(make_pair(*it_id, tfvalues));
    }
    /// Initialize the transferred values of subbasins from other processes,
    ///   NO NEED to create and release in each timestep.
    map<int, float *> recv_transfer_values_map; // key is subbasinID, value is transferred values
    for (auto it_id = upstreams.begin(); it_id != upstreams.end(); ++it_id) {
        for (auto it_up = it_id->second.begin(); it_up != it_id->second.end(); ++it_up) {
            if (subbasin_rank[*it_up] == rank) continue;
            float* tfvalues = nullptr;
            Initialize1DArray(transfer_count, tfvalues, NODATA_VALUE);
            recv_transfer_values_map.insert(make_pair(*it_up, tfvalues));
        }
    }

    MPI_Request request;
    MPI_Status status;

    // Simulation loop
    int sim_loop_num = 0;  /// Simulation loop number according to channel routing time-step
    double t_slope = 0.;   /// Time of hillslope processes
    double t_channel = 0.; /// Time of channel routing processes
    double t_slope_start;
    double t_channel_start;
    for (time_t cur_time = start_time; cur_time <= end_time; cur_time += dt_ch) {
        sim_loop_num += 1;
        if (rank == MASTER_RANK){
            cout << ConvertToString2(&cur_time) << endl;
        }
        int year_idx = GetYear(cur_time) - start_year;
        // Execute by layering orders
        for (int ilyr = 1; ilyr <= max_lyr_id; ilyr++) {
            if (subbsn_layers.find(ilyr) == subbsn_layers.end()) continue;
#ifdef _DEBUG
            cout << ConvertToString2(&cur_time) << ", Rank: " << rank << ", Layer " << ilyr << endl;
#endif
            // 1. Execute hillslope processes
            t_slope_start = TimeCounting();
            for (auto it = subbsn_layers[ilyr].begin(); it != subbsn_layers[ilyr].end(); ++it) {
                int subbasin_id = *it;
#ifdef _DEBUG
                cout << "    Hillslope process, subbasin: " << subbasin_id << endl;
#endif
                ModelMain* psubbasin = model_map[*it];
                for (int i = 0; i < n_hs; i++) {
                    psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                }
            } /* subbsn_layers[ilyr] loop for executing hillslope processes */
            t_slope += TimeCounting() - t_slope_start;
            if (!include_channel) { continue; }

            // 2. Execute channel processes
            t_channel_start = TimeCounting();
            for (auto it = subbsn_layers[ilyr].begin(); it != subbsn_layers[ilyr].end(); ++it) {
                int subbasin_id = *it;
#ifdef _DEBUG
                cout << "    Channel process, subbasin: " << subbasin_id << endl;
#endif
                ModelMain* psubbasin = model_map[*it];

                // 2.1 Set transferred data from upstreams
                for (auto it_upid = upstreams[subbasin_id].begin();
                     it_upid != upstreams[subbasin_id].end(); ++it_upid) {
                    if (subbasin_rank[*it_upid] == rank) {
                        psubbasin->SetTransferredValue(*it_upid, transfer_values_map[*it_upid]);
                    } else {
                        // receive data from the specific rank according to work_tag
                        int work_tag = *it_upid * 10000 + sim_loop_num;;
                        MPI_Irecv(buf, buflen, MPI_FLOAT, subbasin_rank[*it_upid], work_tag, MCW, &request);
                        MPI_Wait(&request, &status);
#ifdef _DEBUG
                        cout << "Receive data of subbasi: " << *it_upid << " from rank: " <<
                                subbasin_rank[*it_upid] << ", tfValues: ";
                        for (int itf = MSG_LEN; itf < buflen; itf++) {
                            cout << buf[itf] << ", ";
                        }
                        cout << endl;
#endif
                        for (int vi = 0; vi < transfer_count; vi++) {
                            recv_transfer_values_map[*it_upid][vi] = buf[MSG_LEN + vi];
                        }
                        psubbasin->SetTransferredValue(*it_upid, recv_transfer_values_map[*it_upid]);
                    }
                }
                psubbasin->StepChannel(cur_time, year_idx);
                psubbasin->AppendOutputData(cur_time);

                // 2.2 If the downstream subbasin is in this process,
                //     there is no need to transfer values to the master process
                int downstream_id = downstream[subbasin_id];
                if (subbasin_rank[downstream_id] == rank) {
                    psubbasin->GetTransferredValue(transfer_values_map[subbasin_id]);
                    continue;
                }
                // 2.3 Otherwise, the transferred values of current subbasin should be sent to another rank
                psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                cout << "Rank: " << rank << ", send subbasinID: " << subbasin_id <<
                        " -> Rank: " << subbasin_rank[downstream_id] << ", tfValues: ";
                for (int itf = MSG_LEN; itf < buflen; itf++) {
                    cout << buf[itf] << ", ";
                }
                cout << endl;
#endif
                int dest_rank = subbasin_rank[downstream[subbasin_id]];
                buf[0] = CVT_FLT(subbasin_id);  // subbasin ID
                buf[1] = CVT_FLT(sim_loop_num); // simulation loop number
                int work_tag = subbasin_id * 10000 + sim_loop_num;
                MPI_Isend(buf, buflen, MPI_FLOAT, dest_rank, work_tag, MCW, &request);
                MPI_Wait(&request, &status);
            } /* subbsn_layers[ilyr] loop for executing channel processes */
            t_channel += TimeCounting() - t_channel_start;
        } /* If subbsn_layers has ilyr */
        MPI_Barrier(MCW);
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
    for (auto it = transfer_values_map.begin(); it != transfer_values_map.end();) {
        if (it->second != nullptr) {
            Release1DArray(it->second);
            it->second = nullptr;
        }
        transfer_values_map.erase(it++);
    }
    for (auto it = recv_transfer_values_map.begin(); it != recv_transfer_values_map.end();) {
        if (it->second != nullptr) {
            Release1DArray(it->second);
            it->second = nullptr;
        }
        recv_transfer_values_map.erase(it++);
    }
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
