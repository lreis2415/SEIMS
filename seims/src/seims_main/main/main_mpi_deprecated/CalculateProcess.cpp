#include "parallel.h"
#include "CombineRaster.h"
#include "utils_time.h"
#include "text.h"

using namespace ccgl::utils_time;

void CalculateProcess(const int world_rank, const int numprocs, const int nslaves,
                      MPI_Comm slave_comm, InputArgs* input_args) {
    double tstart = MPI_Wtime();
    int slave_rank;
    MPI_Comm_rank(slave_comm, &slave_rank);
    StatusMessage(("Enter computing process, worldRank: " + ValueToString(world_rank) +
                      ", slave_rank: " + ValueToString(slave_rank)).c_str());

    MPI_Request request;
    MPI_Status status;

    // receive task information from master process, and scatter to all slave processors
    int* p_task_all = nullptr;
    int* p_updown_ord_all = nullptr;
    int* p_downup_ord_all = nullptr;
    int* p_down_stream_all = nullptr;
    int* p_up_nums_all = nullptr;
    int* p_up_stream_all = nullptr;
    int* p_group_id = nullptr;
    int max_task_len;

    if (world_rank == SLAVE0_RANK) {
        // in this block, the communicator is MCW
        StatusMessage("Receive task information from Master rank...");
        int n_task_all;
        MPI_Recv(&n_task_all, 1, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        p_group_id = new int[nslaves];
        p_task_all = new int[n_task_all];
        p_updown_ord_all = new int[n_task_all];
        p_downup_ord_all = new int[n_task_all];
        p_down_stream_all = new int[n_task_all];
        p_up_nums_all = new int[n_task_all];
        p_up_stream_all = new int[n_task_all * MAX_UPSTREAM];

        MPI_Recv(p_group_id, nslaves, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_task_all, n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_updown_ord_all, n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_downup_ord_all, n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_down_stream_all, n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_up_nums_all, n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(p_up_stream_all, MAX_UPSTREAM * n_task_all, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);

        max_task_len = n_task_all / nslaves;
        StatusMessage("Receive task information from Master rank done!");
    }
    MPI_Bcast(&max_task_len, 1, MPI_INT, 0, slave_comm);
    int group_id;
    /// Subscript of the following variables is the index of subbasins in current slave rank,
    /// NOT the subbasin ID. Be careful when used.
    int* p_tasks = new int[max_task_len];                    ///< subbasin IDs of each task for current rank
    int* p_updown_ord = new int[max_task_len];               ///< Up-down stream order
    int* p_downup_ord = new int[max_task_len];               ///< Down-up stream order
    int* p_down_stream = new int[max_task_len];              ///< Downstream subbasin ID
    int* p_up_nums = new int[max_task_len];                  ///< Upstream subbasins number
    int* p_up_stream = new int[max_task_len * MAX_UPSTREAM]; ///< Upstream subbasin IDs

    MPI_Scatter(p_group_id, 1, MPI_INT, &group_id, 1, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_task_all, max_task_len, MPI_INT, p_tasks, max_task_len, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_updown_ord_all, max_task_len, MPI_INT, p_updown_ord, max_task_len, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_downup_ord_all, max_task_len, MPI_INT, p_downup_ord, max_task_len, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_down_stream_all, max_task_len, MPI_INT, p_down_stream, max_task_len, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_up_nums_all, max_task_len, MPI_INT, p_up_nums, max_task_len, MPI_INT, 0, slave_comm);
    MPI_Scatter(p_up_stream_all, max_task_len * MAX_UPSTREAM, MPI_INT, p_up_stream, max_task_len * MAX_UPSTREAM,
                MPI_INT, 0, slave_comm);

#ifdef _DEBUG
    cout << "Subbasins of slave process " << slave_rank << ":  " << endl;
    cout << "    SubbasinID, UpDownOrder, DownUpOrder, DownStream, UpStreams" << endl;
    for (int i = 0; i < max_task_len; i++) {
        if (p_tasks[i] < 0) continue;
        cout << "    " << p_tasks[i] << ", " << p_updown_ord[i] << ", " <<
            p_downup_ord[i] << ", " << p_down_stream[i] <<", ups:";
        for (int j = 0; j < p_up_nums[i]; j++) {
            cout << p_up_stream[MAX_UPSTREAM * i + j] << " ";
        }
        cout << endl;
    }
#endif /* _DEBUG */

    Release1DArray(p_task_all);
    Release1DArray(p_updown_ord_all);
    Release1DArray(p_downup_ord_all);
    Release1DArray(p_down_stream_all);
    Release1DArray(p_up_nums_all);
    Release1DArray(p_up_stream_all);
    Release1DArray(p_group_id);
    // End of receive and scatter task

    int n_subbasins = 0; // subbasin number of current worldRank
    for (int i = 0; i < max_task_len; i++) {
        if (p_tasks[i] > 0) n_subbasins++;
    }
    if (n_subbasins == 0) {
        cout << "No task for Rank " << world_rank << endl;
        MPI_Abort(MCW, 1);
    }
    double t;
    double t1 = MPI_Wtime();

    ////////////////////////////////////////////////////////////////////
    /// Get module path
    string module_path = GetAppPath();
    // setup model runs for subbasins
    MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
    if (nullptr == mongo_client) {
        throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
    }
    /// Create module factory
    ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args);
    if (nullptr == module_factory) {
        throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
    }
    int tranfer_count = module_factory->GetTransferredInputsCount();
    // Send to master process
    if (slave_rank == 0) {
        MPI_Isend(&tranfer_count, 1, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &request);
        MPI_Wait(&request, &status);
    }

    vector<DataCenterMongoDB *> data_center_list;
    vector<ModelMain *> model_list;
    data_center_list.reserve(n_subbasins);
    model_list.reserve(n_subbasins);
    for (int i = 0; i < n_subbasins; i++) {
        /// Create data center according to subbasin number
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client,
                                                               module_factory, p_tasks[i]);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model = new ModelMain(data_center, module_factory);

        data_center_list.push_back(data_center);
        model_list.push_back(model);
    }
    StatusMessage(("Slave rank " + ValueToString(slave_rank) + " model construct done!").c_str());
    // print IO time
    double t2 = MPI_Wtime();
    t = t2 - t1;
    double* t_receive = new double[nslaves];
    MPI_Gather(&t, 1, MPI_DOUBLE, t_receive, 1, MPI_DOUBLE, 0, slave_comm);
    double load_time = 0.;
    if (slave_rank == 0) {
        load_time = MaxInArray(t_receive, nslaves);
#ifdef _DEBUG
        cout << "[DEBUG]\tTime of reading data -- Max:" << load_time << ", Total:" << Sum(nslaves, t_receive) << "\n";
        cout << "[DEBUG][TIMESPAN][LoadData]" << load_time << endl;
#endif /* _DEBUG */
        cout << "[TIMESPAN][IO]\tINPUT\t" << std::fixed << setprecision(3) << load_time << endl;
    }
    t1 = MPI_Wtime();

    int max_lyr_num = 1;
    /*! Source subbasins in each layer in current slave rank
     * Key: Layering number
     * Value: Source subbasin indexes
     */
    map<int, vector<int> > src_subbsn_layers;
    /*! Non source subbasins in each layer in current slave rank
     * Key: Layering number
     * Value: Non source subbasin indexes
     */
    map<int, vector<int> > non_src_subbsn_layers;
    // Used to find if the downstream subbasin of a finished subbsin is in the same process,
    //   if so, the MPI send operation is not necessary.
    //   the `set` container is more efficient for the 'find' operation
    set<int> down_stream_set;    // Indexes of not-first layer subbasins
    set<int> down_stream_id_set; // Subbasin IDs of not-first layer subbasins
    bool include_channel = false;
    if (model_list[0]->IncludeChannelProcesses()) {
        include_channel = true;
        for (int i = 0; i < n_subbasins; i++) {
            // classification according to the Layering method, i.e., up-down and down-up orders.
            int stream_order = input_args->lyr_mtd == UP_DOWN ? p_updown_ord[i] : p_downup_ord[i];
            if (stream_order > max_lyr_num) { max_lyr_num = stream_order; }
            if (stream_order > 1) {
                // all possible downstream subbasins (i.e., layer >= 2)
                down_stream_set.insert(i);             // index of the array
                down_stream_id_set.insert(p_tasks[i]); // Subbasin ID
            }
            if (p_up_nums[i] == 0) {
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
    cout << "Slave rank: " << slave_rank << "(world rank: " << world_rank << "), Source subbasins of layer 1: ";
    for (auto it = src_subbsn_layers[1].begin(); it != src_subbsn_layers[1].end(); ++it) {
        cout << p_tasks[*it] << ", ";
    }
    cout << endl;
#endif /* _DEBUG */

    double t_task2;
    double t_slope = 0.0;
    double t_channel = 0.0;;
    // Create buffer for passing values across subbasins
    float* buf = nullptr;
    int buflen = MSG_LEN + tranfer_count;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    // Get timesteps
    DataCenterMongoDB* dc = data_center_list[0];
    time_t dt_hs = dc->GetSettingInput()->getDtHillslope();
    time_t dt_ch = dc->GetSettingInput()->getDtChannel();
    time_t cur_time = dc->GetSettingInput()->getStartTime();
    int start_year = GetYear(cur_time);
    int n_hs = CVT_INT(dt_ch / dt_hs);

    // initialize the transferred values, NO NEED to create and release in each timestep.
    map<int, float *> transfer_values_map; // key is subbasinID, value is transferred values
    for (int i = 0; i < n_subbasins; i++) {
        float* tfvalues = nullptr;
        Initialize1DArray(tranfer_count, tfvalues, NODATA_VALUE);
        transfer_values_map.insert(make_pair(p_tasks[i], tfvalues));
    }

    // Simulation loop
    int sim_loop_num = 0; /// Simulation loop number according to channel routing time-step
    for (; cur_time <= dc->GetSettingInput()->getEndTime(); cur_time += dt_ch) {
        sim_loop_num += 1;
        if (slave_rank == 0) StatusMessage(ConvertToString2(&cur_time).c_str());
        int year_idx = GetYear(cur_time) - start_year;
        double t_task1 = MPI_Wtime();
        set<int> done_i_ds; // save subbasin IDs that have been executed
        // Execute by layering orders
        for (int ilyr = 1; ilyr <= max_lyr_num; ilyr++) {
#ifdef _DEBUG
            cout << "slave_rank: " << slave_rank << "(world rank: " << world_rank << "), Layer " << ilyr << endl;
#endif
            // 1. Execute subbasins that does not depend on others
            if (src_subbsn_layers.find(ilyr) != src_subbsn_layers.end()) {
                for (auto it_src = src_subbsn_layers[ilyr].begin(); it_src != src_subbsn_layers[ilyr].end(); ++it_src) {
#ifdef _DEBUG
                    cout << "    layer: " << ilyr << ", source subbasin: " << p_tasks[*it_src] << endl;
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
                    int subbasin_id = p_tasks[*it_src];
                    if (down_stream_id_set.find(p_down_stream[*it_src]) != down_stream_id_set.end()) {
                        psubbasin->GetTransferredValue(transfer_values_map[subbasin_id]);
                        done_i_ds.insert(subbasin_id);
                        continue;
                    }
                    // 1.3 Otherwise, the transferred values of current subbasin should be sent to master rank
                    psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                    cout << "slave rank: " << slave_rank << "(world rank: " << world_rank << "), send subbasinID: " <<
                            subbasin_id << " tfValues: ";
                    for (int itf = MSG_LEN; itf < buflen; itf++) {
                        cout << buf[itf] << ", ";
                    }
                    cout << endl;
#endif
                    buf[0] = 1.f;                  // message type: Send subbasin data to master rank
                    buf[1] = CVT_FLT(subbasin_id); // subbasin ID
                    buf[2] = CVT_FLT(t);           // time
                    buf[3] = CVT_FLT(sim_loop_num);// simulation loop number
                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                } /* src_subbsn_layers[iLyr] loop */
            }     /* If src_subbsn_layers has iLyr */

            // 2. Execute subbasins that depend on others
            if (non_src_subbsn_layers.find(ilyr) == non_src_subbsn_layers.end()) { continue; }
            // some layers may absent in current slave rank
            if (!include_channel) { continue; }

            // 2.1 Execute hillslope processes
            set<int> todo_set; // To be executed subbasin indexes of current layer
            for (auto it_down = non_src_subbsn_layers[ilyr].begin();
                 it_down != non_src_subbsn_layers[ilyr].end(); ++it_down) {
#ifdef _DEBUG
                cout << "    layer: " << ilyr << ", non-source subbasin(hillslope): " << p_tasks[*it_down] << endl;
#endif
                ModelMain* psubbasin = model_list[*it_down];
                for (int i = 0; i < n_hs; i++) {
                    psubbasin->StepHillSlope(cur_time + i * dt_hs, year_idx, i);
                }
                todo_set.insert(*it_down);
            }
            t_task2 = MPI_Wtime();
            t_slope += t_task2 - t_task1;
            // TODO, tSlospe currently also contains the time of channel processes of source subbasins
            t_task1 = MPI_Wtime();

            // 2.2 The channel routing of downStream subbasin will be calculated
            //     if its upstream subbasins are already calculated
            set<int> cando_set;
            while (!todo_set.empty()) {
                // 2.2.1 Find all subbasins that the channel routing can be done
                //       without asking the master process for data
                for (auto it_todo = todo_set.begin(); it_todo != todo_set.end();) {
                    bool up_finished = true;
                    for (int j = 0; j < p_up_nums[*it_todo]; j++) {
                        int up_id = p_up_stream[*it_todo * MAX_UPSTREAM + j];
                        // if can not find upstreams, this subbasin can not be done
                        if (done_i_ds.find(up_id) == done_i_ds.end()) {
                            up_finished = false;
                            break;
                        }
                    }
                    if (up_finished) {
                        cando_set.insert(*it_todo);
                        todo_set.erase(it_todo++);
                    } else {
                        ++it_todo;
                    }
                }

#ifdef _DEBUG
                cout << "slave_rank " << slave_rank << "(world rank: " << world_rank << ") todo set (subbasinID): ";
                for (auto it_todo = todo_set.begin(); it_todo != todo_set.end(); ++it_todo) {
                    cout << p_tasks[*it_todo] << ", ";
                }
                cout << ", cando set (subbasinID): ";
                for (auto it_cando = cando_set.begin(); it_cando != cando_set.end(); ++it_cando) {
                    cout << p_tasks[*it_cando] << ", ";
                }
                cout << endl;
#endif
                // 2.2.2 If can not find subbasins to calculate according to local information,
                //       ask the master process if there are new upstream subbasins calculated
                if (cando_set.empty()) {
                    buf[0] = 2.f; // Ask for the already calculated subbasins from other slave ranks
                    buf[1] = CVT_FLT(group_id);
                    buf[2] = CVT_FLT(world_rank);
                    buf[3] = CVT_FLT(sim_loop_num); // simulation loop number

                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                    // Wait and receive data from master rank
                    int msg_len;
                    MPI_Irecv(&msg_len, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    MPI_Wait(&request, &status);

                    float* pdata = new float[msg_len];
                    MPI_Irecv(pdata, msg_len, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
#ifdef _DEBUG
                    cout << "recv count of subbasin data: " << msg_len / (tranfer_count + 1) << ", subbasinIDs: ";
                    for (int ri = 0; ri < msg_len / (tranfer_count + 1); ri++) {
                        cout << CVT_INT(pdata[ri * (tranfer_count + 1)]) << ", ";
                    }
                    cout << endl;
#endif
                    for (int ri = 0; ri < msg_len / (tranfer_count + 1); ri++) {
                        int recv_subid = CVT_INT(pdata[ri * (tranfer_count + 1)]);
                        if (transfer_values_map.find(recv_subid) == transfer_values_map.end()) {
                            float* tfvalues = nullptr;
                            Initialize1DArray(tranfer_count, tfvalues, NODATA_VALUE);
                            transfer_values_map.insert(make_pair(recv_subid, tfvalues));
                        }
                        for (int vi = 0; vi < tranfer_count; vi++) {
                            transfer_values_map[recv_subid][vi] = pdata[ri * (tranfer_count + 1) + vi + 1];
                        }
                        done_i_ds.insert(recv_subid);
                    }
                    Release1DArray(pdata);
                } else {
                    // 2.2.3 The required subbasin data has been satisfied,
                    //       execute subbasin in cando_set, and send data to master rank if necessary
                    for (auto it_cando = cando_set.begin(); it_cando != cando_set.end();) {
#ifdef _DEBUG
                        cout << "    layer: " << ilyr << ", non-source subbasin(channel): " <<
                            p_tasks[*it_cando] << endl;
#endif
                        ModelMain* psubbasin = model_list[*it_cando];
                        for (int j = 0; j < p_up_nums[*it_cando]; j++) {
                            int up_id = p_up_stream[*it_cando * MAX_UPSTREAM + j];
                            psubbasin->SetTransferredValue(up_id, transfer_values_map[up_id]);
                        }
                        psubbasin->StepChannel(cur_time, year_idx);
                        psubbasin->AppendOutputData(cur_time);

                        if (down_stream_id_set.find(p_down_stream[*it_cando]) != down_stream_id_set.end()) {
                            psubbasin->GetTransferredValue(transfer_values_map[p_tasks[*it_cando]]);
                            done_i_ds.insert(p_tasks[*it_cando]);
                            cando_set.erase(it_cando++); // remove current subbasin from cando_set
                            continue;
                        }
                        // transfer the result to the master process
                        buf[0] = 1.f;                         // message type: Send subbasin data to master rank
                        buf[1] = CVT_FLT(p_tasks[*it_cando]); // subbasin ID
                        buf[2] = CVT_FLT(t);
                        buf[3] = CVT_FLT(sim_loop_num);       // simulation loop number
                        psubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                        cout << "slave rank: " << slave_rank << "(world rank: " << world_rank << "), send subbasinID: "
                                << p_tasks[*it_cando] << ", tfValues: ";
                        for (int itf = MSG_LEN; itf < buflen; itf++) {
                            cout << buf[itf] << ", ";
                        }
                        cout << endl;
#endif
                        MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                        MPI_Wait(&request, &status);
                        cando_set.erase(it_cando++);
                    } /* cando_set loop of current layer */
                }     /* cando_set is not empty */
            }         /* while todo_set is not empty */

            t_task2 = MPI_Wtime();
            t_channel += t_task2 - t_task1;
        } /* subbasin layers loop */

        MPI_Barrier(slave_comm);
        if (slave_rank == 0) {
            buf[0] = 0.f; // signal for new timestep
            MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
        }
    } /* timestep loop */

    t2 = MPI_Wtime();
    t = t2 - t1;
    MPI_Gather(&t, 1, MPI_DOUBLE, t_receive, 1, MPI_DOUBLE, 0, slave_comm);
    // TODO, Gather and scatter the computing time of each modules from all slave processors
    if (slave_rank == 0) {
        double computing_time = MaxInArray(t_receive, nslaves);
#ifdef _DEBUG
        cout << "[DEBUG][TIMESPAN][COMPUTING]\tnprocs: " << numprocs - 1 << ", Max: " << computing_time
                << ", Total: " << Sum(nslaves, t_receive) << "\n";
#endif
        cout << "[TIMESPAN][COMPUTING]\tALL\t" << std::fixed << setprecision(3) << computing_time << endl;
    }
    /***************  Outputs and combination  ***************/
    t1 = MPI_Wtime();
    for (int i = 0; i < n_subbasins; i++) {
        model_list[i]->Output();
    }
    t2 = MPI_Wtime();
    t = t2 - t1;

    /*** Combine raster outputs serially by one processor. ***/
    /// The operation could be considered as post-process,
    ///   therefore, the time-consuming is not included.
    if (slave_rank == 0) {
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

    MPI_Gather(&t, 1, MPI_DOUBLE, t_receive, 1, MPI_DOUBLE, 0, slave_comm);
    if (slave_rank == 0) {
        cout << "[TIMESPAN][IO]\tOUTPUT\t" << std::fixed << setprecision(3) << t << endl;
        cout << "[TIMESPAN][IO]\tALL\t" << std::fixed << setprecision(3) << t + load_time << endl;
    }

    double t_end = MPI_Wtime();
    t = t_end - tstart;
    MPI_Gather(&t, 1, MPI_DOUBLE, t_receive, 1, MPI_DOUBLE, 0, slave_comm);
    if (slave_rank == 0) {
        double all_time = MaxInArray(t_receive, nslaves);
        cout << "[TIMESPAN][CALCULATION]\tALL\t" << all_time << "\n";
    }
    MPI_Barrier(slave_comm);

    // tell the master process to exit
    if (slave_rank == 0) {
        buf[0] = 9.f;
        MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
        MPI_Wait(&request, &status);
    }

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
    Release1DArray(t_receive);

    Release1DArray(p_tasks);
    Release1DArray(p_updown_ord);
    Release1DArray(p_downup_ord);
    Release1DArray(p_down_stream);
    Release1DArray(p_up_nums);
    Release1DArray(p_up_stream);
}
