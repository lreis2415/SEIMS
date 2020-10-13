#include "CalculateProcess.h"

#include <map>
#include <vector>

#include "utils_time.h"
#include "DataCenterMongoDB.h"
#include "invoke.h"
#include "ModelMain.h"
#include "CombineRaster.h"
#include "text.h"
#include "Logging.h"

#include "parallel.h"
#include "TaskInformation.h"
#include "LoadParallelTasks.h"

using namespace utils_time;
using namespace utils_array;
using std::map;
using std::vector;

void CalculateProcess(InputArgs* input_args, const int rank, const int size,
                      mongoc_client_pool_t* mongo_pool /* = nullptr */) {
    LOG(TRACE) << "Computing process, Rank: " << rank;
    double tstart = MPI_Wtime();
    /// Get module path, i.e., the path of executable and dynamic libraries
    string module_path = GetAppPath();

    /// Load parallel task scheduling information and record time-consuming.
    TaskInfo* task_info = new TaskInfo(size, rank);
    mongoc_client_t* mclient = nullptr;
    MongoClient* mongo_client = nullptr;
    MongoGridFs* spatial_gfs_in = nullptr;
    MongoGridFs* spatial_gfs_out = nullptr;
    if (nullptr == mongo_pool) {
        mongo_client = new MongoClient(input_args->host.c_str(), input_args->port);
    } else {
        // The total number of clients that can be created from this pool is limited
        //   by the URI option ¡°maxPoolSize¡±, default 100.
        mclient = mongoc_client_pool_pop(mongo_pool);
        if (!mclient) {
            throw ModelException("MongoDBClient", "Constructor",
                                 "Failed to pop mongoClient from mongoc_client_pool!");
        }
        mongo_client = new MongoClient(mclient);
    }
    spatial_gfs_in = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_SPATIAL));
    spatial_gfs_out = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
    if (!spatial_gfs_in || !spatial_gfs_out) {
        LOG(TRACE) << "MongoDB GridFS initialized failed, the program will be terminated!";
        if (mongo_pool && mclient) {
            mongoc_client_pool_push(mongo_pool, mclient);
        } else {
            mongo_client->Destroy();
            delete mongo_client;
        }
        MPI_Abort(MCW, 1);
    }

    LoadTasks(mongo_client, input_args, size, rank, task_info);
    if (!task_info->Build()) {
        LOG(TRACE) << "Rank: " << rank << ", task information build failed!";
        MPI_Abort(MCW, 1);
    }
    double t_load_task = MPI_Wtime() - tstart;

    /******************* Actual calculation *******************/
    tstart = MPI_Wtime();                             /// Start to construct objects of subbasin models
    int n_subbasins = task_info->GetSubbasinNumber(); // subbasin number of current rank
    if (n_subbasins == 0) {
        LOG(TRACE) << "No task for rank " << rank;
        MPI_Abort(MCW, 1);
    }
    int max_lyr_id_all = task_info->GetGlobalMaxLayerID(); /// Global maximum layering ID

    /// Create module factory
    ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args, rank, size);
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
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client, spatial_gfs_in, spatial_gfs_out,
                                                               module_factory, *it_id);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model = new ModelMain(data_center, module_factory);
#ifdef HAS_VARIADIC_TEMPLATES
        data_center_map.emplace(*it_id, data_center);
        model_map.emplace(*it_id, model);
#else
        data_center_map.insert(make_pair(*it_id, data_center));
        model_map.insert(make_pair(*it_id, model));
#endif
    }
    /// Specific handling code, which maybe improved in the future version.
    ///   S1: SetSlopeCoefofBasin(), the algorithm is coincident with `clsSubbasins::Subbasin2Basin()`.
    int unit_count = 0;
    float slope_sum = 0.f;
    for (auto it_data = data_center_map.begin(); it_data != data_center_map.end(); ++it_data) {
        map<int, Subbasin *>& subbsn_objs = it_data->second->GetSubbasinData()->GetSubbasinObjects();
        for (auto it_subbsn = subbsn_objs.begin(); it_subbsn != subbsn_objs.end(); ++it_subbsn) {
            int tmp_count = it_subbsn->second->GetCellCount();
            slope_sum += tmp_count * atan(it_subbsn->second->GetSlope());
            unit_count += tmp_count;
        }
    }
    int unit_count_all = 0;
    float slope_sum_all = 0.f;
    MPI_Allreduce(&unit_count, &unit_count_all, 1, MPI_INT, MPI_SUM, MCW);
    MPI_Allreduce(&slope_sum, &slope_sum_all, 1, MPI_FLOAT, MPI_SUM, MCW);
    float slope_basin = tan(slope_sum_all / unit_count_all);

    for (auto it_data = data_center_map.begin(); it_data != data_center_map.end(); ++it_data) {
        map<int, Subbasin *>& subbsn_objs = it_data->second->GetSubbasinData()->GetSubbasinObjects();
        for (auto it_subbsn = subbsn_objs.begin(); it_subbsn != subbsn_objs.end(); ++it_subbsn) {
            Subbasin* tmp_subbsn = it_subbsn->second;
            tmp_subbsn->SetSlopeCoefofBasin(tmp_subbsn->GetSlope() / slope_basin);
        }
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
    if (time_slice < 1) time_slice = 1;
    int multiplier = CVT_INT(ceil(CVT_FLT(max_time_slices) / CVT_FLT(time_slice)));
    if (input_args->skd_mtd == SPATIAL) multiplier = 1;

    task_info->MallocTransferredValues(transfer_count, multiplier);
    /// Transferred values of subbasins in current rank with timestep stamp
    map<int, map<int, float *> >& ts_subbsn_tf_values = task_info->GetSubbasinTransferredValues();
    /// Record the actual simulation loop number of each subbasin
    map<int, int> ts_subbsn_loop;
    /// Received transferred values of subbasins in current rank with timestep stamp
    map<int, map<int, float *> >& recv_ts_subbsn_tf_values = task_info->GetReceivedSubbasinTransferredValues();

    /// Reduce for model constructing time, which also input time
    double t_model_construct = MPI_Wtime() - tstart;
    LOG(TRACE) << "Rank " << rank << " construct models done!";

    MPI_Request request;
    MPI_Status status;

    // Simulation loop
    double t_slope = 0.;         ///< Time of hillslope processes
    double t_channel = 0.;       ///< Time of channel routing processes
    double t_barrier = 0.;       ///< Time of MPI barrier
    double t_slope_start = 0.;   ///< Temporary variables to counting time of hillslope processes
    double t_channel_start = 0.; ///< Temporary variables to counting time of channel processes
    double t_barrier_start = 0.; ///< Temporary variables to counting time of MPI barrier

    int sim_loop_num = 0; /// Simulation loop number, which will be ciculated at 1 ~ max_lyr_id_all
    int act_loop_num = 0; /// Actual simulation loop number, which will be 1 ~ N
    int exec_lyr_num = 1; /// For SPATIAL scheduling method
    int max_loop_num = max_lyr_id_all * multiplier;
    tstart = MPI_Wtime(); /// Start simulation
    int pre_year_idx = -1;
    for (time_t ts = start_time; ts <= end_time; ts += dt_ch) {
        sim_loop_num += 1;
        act_loop_num += 1;
        int year_idx = GetYear(ts) - start_year;
        if (rank == MASTER_RANK) {
            if (pre_year_idx != year_idx) {
                LOG(DEBUG) << "  Simulation year: " << start_year + year_idx;
            }
            LOG(DEBUG) << ConvertToString2(ts);
        }
        // Execute by layering orders
        for (int ilyr = 1; ilyr <= max_lyr_id_all; ilyr++) {
            // if (subbsn_layers.find(ilyr) == subbsn_layers.end()) continue; // DO NOT UNCOMMENT THIS!

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
                    // If subbasin_id of the actual loop already executed, continue.
                    if (ts_subbsn_loop[subbasin_id] >= act_loop_num + lyr_dlt) {
                        continue;
                    }
                    // 1. Execute hillslope processes
                    t_slope_start = MPI_Wtime();
                    ModelMain* psubbasin = model_map[*it];
                    for (int i = 0; i < n_hs; i++) {
                        psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                    }
                    t_slope += MPI_Wtime() - t_slope_start;
                    if (!include_channel) continue;

                    // 2. Execute channel processes
                    t_channel_start = MPI_Wtime();

                    // 2.1 Set transferred data from upstreams
                    for (auto it_upid = upstreams[subbasin_id].begin();
                         it_upid != upstreams[subbasin_id].end(); ++it_upid) {
                        if (subbasin_rank[*it_upid] == rank) {
                            psubbasin->SetTransferredValue(*it_upid, ts_subbsn_tf_values[cur_sim_loop_num][*it_upid]);
                        } else {
                            // receive data from the specific rank according to work_tag
                            int work_tag = *it_upid * 10000 + cur_sim_loop_num;;
                            MPI_Irecv(buf, buflen, MPI_FLOAT, subbasin_rank[*it_upid], work_tag, MCW, &request);
                            MPI_Wait(&request, &status);

                            for (int vi = 0; vi < transfer_count; vi++) {
                                recv_ts_subbsn_tf_values[cur_sim_loop_num][*it_upid][vi] = buf[MSG_LEN + vi];
                            }
                            psubbasin->SetTransferredValue(*it_upid,
                                                           recv_ts_subbsn_tf_values[cur_sim_loop_num][*it_upid]);
                        }
                    }
                    psubbasin->StepChannel(cur_time, year_idx);
                    psubbasin->AppendOutputData(cur_time);
                    ts_subbsn_loop[subbasin_id] = act_loop_num + lyr_dlt;

                    // 2.2 If the downstream subbasin is in this process,
                    //     there is no need to transfer values to the master process
                    int downstream_id = downstream[subbasin_id];
                    if (downstream_id > 0 && subbasin_rank[downstream_id] == rank) {
                        psubbasin->GetTransferredValue(ts_subbsn_tf_values[cur_sim_loop_num][subbasin_id]);
                        t_channel += MPI_Wtime() - t_channel_start;
                        continue;
                    }
                    if (downstream_id < 0) {
                        // There is no need to get transferred values
                        t_channel += MPI_Wtime() - t_channel_start;
                        continue;
                    }
                    // 2.3 Otherwise, the transferred values of current subbasin should be sent to another rank
                    psubbasin->GetTransferredValue(&buf[MSG_LEN]);

                    int dest_rank = subbasin_rank[downstream[subbasin_id]];
                    buf[0] = CVT_FLT(subbasin_id);      // subbasin ID
                    buf[1] = CVT_FLT(cur_sim_loop_num); // simulation loop number
                    int work_tag = subbasin_id * 10000 + cur_sim_loop_num;
                    MPI_Isend(buf, buflen, MPI_FLOAT, dest_rank, work_tag, MCW, &request);
                    MPI_Wait(&request, &status);
                    t_channel += MPI_Wtime() - t_channel_start;
                } /* subbsn_layers[cur_ilyr] loop */
            }     /* loop of lyr_dlt = 0 to exec_lyr_num */
        }         /* If subbsn_layers has ilyr */

        if (sim_loop_num % max_loop_num == 0) {
            sim_loop_num = 0;
            t_barrier_start = MPI_Wtime();
            MPI_Barrier(MCW);
            t_barrier += MPI_Wtime() - t_barrier_start;
        }
        pre_year_idx = year_idx;
    } /* timestep loop */
    double t_comp = MPI_Wtime() - tstart;

    /***************  Outputs ***************/
    tstart = MPI_Wtime();
    for (auto it_id = rank_subbsn_ids.begin(); it_id != rank_subbsn_ids.end(); ++it_id) {
        model_map[*it_id]->Output();
    }
    double t_output = MPI_Wtime() - tstart;

    /***************  Counting time ***************/
    double load_task_tmax;  ///< Maximum time-consuming of loading parallel tasks in all ranks
    double model_cons_tmax; ///< Maximum time-consuming of constructing models in all ranks
    double slope_tmax;      ///< Maximum time-consuming of hillslope processes in all ranks
    double channel_tmax;    ///< Maximum time-consuming of channel processes in all ranks
    double barrier_tmax;    ///< Maximum time-consuming of MPI barrier in all ranks
    double comp_tmax;       ///< Maximum time-consuming of actual computing in all ranks
    double output_tmax;     ///< Maximum time-consuming of model outputing in all ranks
    MPI_Reduce(&t_load_task, &load_task_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_model_construct, &model_cons_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_slope, &slope_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_channel, &channel_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_barrier, &barrier_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_comp, &comp_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    MPI_Reduce(&t_output, &output_tmax, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MCW);
    double input_tmax = load_task_tmax + model_cons_tmax;
    double io_tmax = input_tmax + output_tmax;
    double all_tmax = io_tmax + comp_tmax;

    double load_task_tmin;  ///< Minimum time-consuming of loading parallel tasks in all ranks
    double model_cons_tmin; ///< Minimum time-consuming of constructing models in all ranks
    double slope_tmin;      ///< Minimum time-consuming of hillslope processes in all ranks
    double channel_tmin;    ///< Minimum time-consuming of channel processes in all ranks
    double barrier_tmin;    ///< Minimum time-consuming of MPI barrier in all ranks
    double comp_tmin;       ///< Minimum time-consuming of actual computing in all ranks
    double output_tmin;     ///< Minimum time-consuming of model outputing in all ranks
    MPI_Reduce(&t_load_task, &load_task_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_model_construct, &model_cons_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_slope, &slope_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_channel, &channel_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_barrier, &barrier_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_comp, &comp_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    MPI_Reduce(&t_output, &output_tmin, 1, MPI_DOUBLE, MPI_MIN, MASTER_RANK, MCW);
    double input_tmin = load_task_tmin + model_cons_tmin;
    double io_tmin = input_tmin + output_tmin;
    double all_tmin = io_tmin + comp_tmin;

    double load_task_tavg;  ///< Average time-consuming of loading parallel tasks in all ranks
    double model_cons_tavg; ///< Average time-consuming of constructing models in all ranks
    double slope_tavg;      ///< Average time-consuming of hillslope processes in all ranks
    double channel_tavg;    ///< Average time-consuming of channel processes in all ranks
    double barrier_tavg;    ///< Average time-consuming of MPI barrier in all ranks
    double comp_tavg;       ///< Average time-consuming of actual computing in all ranks
    double output_tavg;     ///< Average time-consuming of model outputing in all ranks
    MPI_Reduce(&t_load_task, &load_task_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_model_construct, &model_cons_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_slope, &slope_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_channel, &channel_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_barrier, &barrier_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_comp, &comp_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    MPI_Reduce(&t_output, &output_tavg, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK, MCW);
    load_task_tavg /= size;
    model_cons_tavg /= size;
    slope_tavg /= size;
    channel_tavg /= size;
    barrier_tavg /= size;
    comp_tavg /= size;
    output_tavg /= size;
    double input_tavg = load_task_tavg + model_cons_tavg;
    double io_tavg = input_tavg + output_tavg;
    double all_tavg = io_tavg + comp_tavg;

    if (rank == MASTER_RANK) {
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][COMP][Slope]   " << std::fixed << setprecision(3) << slope_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][COMP][Channel] " << std::fixed << setprecision(3) << channel_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][COMP][Barrier] " << std::fixed << setprecision(3) << barrier_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][COMP][ALL]     " << std::fixed << setprecision(3) << comp_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][IO  ][Input]   " << std::fixed << setprecision(3) << input_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][IO  ][Output]  " << std::fixed << setprecision(3) << output_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][IO  ][ALL]     " << std::fixed << setprecision(3) << io_tmax;
        CLOG(INFO, LOG_TIMESPAN) << "[MAX][SIMU][ALL]     " << std::fixed << setprecision(3) << all_tmax;

        CLOG(INFO, LOG_TIMESPAN) << "[MIN][COMP][Slope]   " << std::fixed << setprecision(3) << slope_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][COMP][Channel] " << std::fixed << setprecision(3) << channel_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][COMP][Barrier] " << std::fixed << setprecision(3) << barrier_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][COMP][ALL]     " << std::fixed << setprecision(3) << comp_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][IO  ][Input]   " << std::fixed << setprecision(3) << input_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][IO  ][Output]  " << std::fixed << setprecision(3) << output_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][IO  ][ALL]     " << std::fixed << setprecision(3) << io_tmin;
        CLOG(INFO, LOG_TIMESPAN) << "[MIN][SIMU][ALL]     " << std::fixed << setprecision(3) << all_tmin;

        CLOG(INFO, LOG_TIMESPAN) << "[AVG][COMP][Slope]   " << std::fixed << setprecision(3) << slope_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][COMP][Channel] " << std::fixed << setprecision(3) << channel_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][COMP][Barrier] " << std::fixed << setprecision(3) << barrier_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][COMP][ALL]     " << std::fixed << setprecision(3) << comp_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][IO  ][Input]   " << std::fixed << setprecision(3) << input_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][IO  ][Output]  " << std::fixed << setprecision(3) << output_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][IO  ][ALL]     " << std::fixed << setprecision(3) << io_tavg;
        CLOG(INFO, LOG_TIMESPAN) << "[AVG][SIMU][ALL]     " << std::fixed << setprecision(3) << all_tavg;
    }

    /*** Combine raster outputs serially by one processor. ***/
    /// The operation could be considered as post-process,
    ///   therefore, the time-consuming is not included.
    if (rank == MASTER_RANK) {
        CLOG(TRACE, LOG_OUTPUT) << "Combining raster data to GeoTIFF file(s)...";
        //MongoGridFs* gfs = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput* outputs = data_center_map.begin()->second->GetSettingOutput();
        for (auto it = outputs->m_printInfos.begin(); it != outputs->m_printInfos.end(); ++it) {
            for (auto item_it = (*it)->m_PrintItems.begin(); item_it != (*it)->m_PrintItems.end(); ++item_it) {
                PrintInfoItem* item = *item_it;
                if (item->m_nLayers < 1) continue;
                // Only need to handle raster data
                CLOG(TRACE, LOG_OUTPUT) << "Combining raster: " << item->Corename << "...";
                if (!CombineRasterResultsMongo(spatial_gfs_out, item->Corename,
                                               data_center_map.begin()->second->GetSubbasinsCount(),
                                               data_center_map.begin()->second->GetOutputScenePath(),
                                               input_args->scenario_id, input_args->calibration_id)) {
                    CLOG(WARNING, LOG_OUTPUT) << "Combining raster: " << item->Corename << " FAILED!";
                } else {
                    CLOG(TRACE, LOG_OUTPUT) << "Combining raster: " << item->Corename << " SUCCEED!";
                }
            }
        }
        //delete gfs;
    }
    /*** End of Combine raster outputs. ***/

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
    if (spatial_gfs_in) {
        delete spatial_gfs_in;
        spatial_gfs_in = nullptr;
    }
    if (spatial_gfs_out != nullptr) {
        delete spatial_gfs_out;
        spatial_gfs_out = nullptr;
    }
    if (mongo_pool && mclient) {
        mongoc_client_pool_push(mongo_pool, mclient);
    } else {
        mongo_client->Destroy();
        delete mongo_client;
    }
    delete task_info;
    if (buf != nullptr) Release1DArray(buf);
}
