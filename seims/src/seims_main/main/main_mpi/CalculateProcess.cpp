#include "parallel.h"
#include "CombineRaster.h"

void CalculateProcess(int world_rank, int numprocs, int nSlaves, MPI_Comm slaveComm, InputArgs *input_args) {
    double tStart = MPI_Wtime();
    int slaveRank;
    MPI_Comm_rank(slaveComm, &slaveRank);
    StatusMessage(("Enter computing process, world_rank: " + ValueToString(world_rank) +
        ", slave_rank: " + ValueToString(slaveRank)).c_str());

    MPI_Request request;
    MPI_Status status;

    // receive task information from master process, and scatter to all slave processors
    int *pTaskAll = nullptr;
    int *pUpdownOrdAll = nullptr;
    int *pDownupOrdAll = nullptr;
    int *pDownStreamAll = nullptr;
    int *pUpNumsAll = nullptr;
    int *pUpStreamAll = nullptr;
    int *pGroupId = nullptr;
    int maxTaskLen;

    if (world_rank == SLAVE0_RANK) {  // in this block, the communicator is MCW
        StatusMessage("Receive task information from Master rank...");
        int nTaskAll;
        MPI_Recv(&nTaskAll, 1, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        pGroupId = new int[nSlaves];
        pTaskAll = new int[nTaskAll];
        pUpdownOrdAll = new int[nTaskAll];
        pDownupOrdAll = new int[nTaskAll];
        pDownStreamAll = new int[nTaskAll];
        pUpNumsAll = new int[nTaskAll];
        pUpStreamAll = new int[nTaskAll * MAX_UPSTREAM];

        MPI_Recv(pGroupId, nSlaves, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pTaskAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pUpdownOrdAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pDownupOrdAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pDownStreamAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pUpNumsAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);
        MPI_Recv(pUpStreamAll, MAX_UPSTREAM * nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &status);

        maxTaskLen = nTaskAll / nSlaves;
        StatusMessage("Receive task information from Master rank done!");
    }
    MPI_Bcast(&maxTaskLen, 1, MPI_INT, 0, slaveComm);
    int groupId;
    int *pTasks = new int[maxTaskLen]; ///< subbasin IDs of each task for current rank
    int *pUpdownOrd = new int[maxTaskLen];
    int *pDownupOrd = new int[maxTaskLen];
    int *pDownStream = new int[maxTaskLen];
    int *pUpNums = new int[maxTaskLen];
    int *pUpStream = new int[maxTaskLen * MAX_UPSTREAM];

    MPI_Scatter(pGroupId, 1, MPI_INT, &groupId, 1, MPI_INT, 0, slaveComm);
    MPI_Scatter(pTaskAll, maxTaskLen, MPI_INT, pTasks, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pUpdownOrdAll, maxTaskLen, MPI_INT, pUpdownOrd, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pDownupOrdAll, maxTaskLen, MPI_INT, pDownupOrd, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pDownStreamAll, maxTaskLen, MPI_INT, pDownStream, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pUpNumsAll, maxTaskLen, MPI_INT, pUpNums, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pUpStreamAll, maxTaskLen * MAX_UPSTREAM, MPI_INT, pUpStream, maxTaskLen * MAX_UPSTREAM,
                MPI_INT, 0, slaveComm);

#ifdef _DEBUG
    cout << "Subbasins of slave process " << slaveRank << ":  " << endl;
    for (int i = 0; i < maxTaskLen; i++) {
        if (pTasks[i] < 0) continue;
        cout << pTasks[i] << " " << pUpdownOrd[i] << " " << pDownStream[i] << ", ups:";
        for (int j = 0; j < pUpNums[i]; j++) {
            cout << pUpStream[MAX_UPSTREAM * i + j] << " ";
        }
        cout << endl;
    }
#endif /* _DEBUG */

    Release1DArray(pTaskAll);
    Release1DArray(pUpdownOrdAll);
    Release1DArray(pDownupOrdAll);
    Release1DArray(pDownStreamAll);
    Release1DArray(pUpNumsAll);
    Release1DArray(pUpStreamAll);
    Release1DArray(pGroupId);
    // End of receive and scatter task

    int nSubbasins = 0;  // subbasin number of current world_rank
    for (int i = 0; i < maxTaskLen; i++) {
        if (pTasks[i] > 0) nSubbasins++;
    }
    if (nSubbasins == 0) {
        cout << "No task for Rank " << world_rank << endl;
        MPI_Abort(MCW, 1);
    }
    double t1, t2;
    double t;
    t1 = MPI_Wtime();

    ////////////////////////////////////////////////////////////////////
    /// Get module path
    string modulePath = GetAppPath();
    // setup model runs for subbasins
    MongoClient *mongoClient = MongoClient::Init(input_args->m_host_ip, input_args->m_port);
    if (nullptr == mongoClient) {
        throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
    }
    /// Create module factory
    ModuleFactory *moduleFactory = ModuleFactory::Init(modulePath, input_args);
    if (nullptr == moduleFactory) {
        throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
    }
    int tranferCount = moduleFactory->GetTransferredInputsCount();
    // Send to master process
    if (slaveRank == 0) {
        MPI_Isend(&tranferCount, 1, MPI_INT, MASTER_RANK, WORK_TAG, MCW, &request);
        MPI_Wait(&request, &status);
    }

    vector<DataCenterMongoDB *> dataCenterList;
    vector<ModelMain *> modelList;
    dataCenterList.reserve(nSubbasins);
    modelList.reserve(nSubbasins);
    for (int i = 0; i < nSubbasins; i++) {
        /// Create data center according to subbasin number
        DataCenterMongoDB *dataCenter = new DataCenterMongoDB(input_args, mongoClient, moduleFactory, pTasks[i]);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain *model = new ModelMain(dataCenter, moduleFactory);

        dataCenterList.push_back(dataCenter);
        modelList.push_back(model);
    }
    StatusMessage(("Slave rank " + ValueToString(slaveRank) + " model construct done!").c_str());
    // print IO time
    t2 = MPI_Wtime();
    t = t2 - t1;
    double *tReceive = new double[nSlaves];
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    double loadTime = 0.;
    if (slaveRank == 0) {
        loadTime = Max(tReceive, nSlaves);
#ifdef _DEBUG
        cout << "[DEBUG]\tTime of reading data -- Max:" << loadTime << ", Total:" << Sum(nSlaves, tReceive) << "\n";
        cout << "[DEBUG][TIMESPAN][LoadData]" << loadTime << endl;
#endif /* _DEBUG */
        cout << "[TIMESPAN][IO]\tINPUT\t" << fixed << setprecision(3) << loadTime << endl;
    }
    t1 = MPI_Wtime();

    vector<int> sourceBasins;  // Index of source subbasins in current processor
    // Used to find if the downstream subbasin of a finished subbsin is in the same process,
    //   if so, the MPI send operation is not necessary.
    //   the `set` container is more efficient for the 'find' operation
    set<int> downStreamSet;  // index of not source subbasins
    set<int> downStreamIdSet; // ID of not source subbasins
    bool includeChannel = false;
    if (modelList[0]->IncludeChannelProcesses()) {
        includeChannel = true;
        for (int i = 0; i < nSubbasins; i++) {
            // classification according to the Layering method, i.e., up-down and down-up orders.
            int stream_order = input_args->m_layer_mtd == UP_DOWN ? pUpdownOrd[i] : pDownupOrd[i];
            if (stream_order == 1) {
                sourceBasins.push_back(i);
            } else {
                downStreamSet.insert(i); //index of the array
                downStreamIdSet.insert(pTasks[i]); //id of subbasin
            }
        }
    } else { // if no channel processes are simulated
        for (int i = 0; i < nSubbasins; i++) {
            sourceBasins.push_back(i);
        }
    }
#ifdef _DEBUG
    cout << "Slave rank: " << slaveRank << "(world rank: " << world_rank << "), Source subbasins index: ";
    for (auto it = sourceBasins.begin(); it != sourceBasins.end(); it++) {
        cout << *it << ", ";
    }
    cout << endl;
#endif /* _DEBUG */
    double tTask1, tTask2;
    double tSlope = 0.0;
    double tChannel = 0.0;;
    // Create buffer for passing values across subbasins
    float *buf = nullptr;
    int buflen = MSG_LEN + tranferCount;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    // time loop
    DataCenterMongoDB *dc = dataCenterList[0];
    time_t dtHs = dc->getSettingInput()->getDtHillslope();
    time_t dtCh = dc->getSettingInput()->getDtChannel();
    time_t curTime = dc->getSettingInput()->getStartTime();
    int startYear = GetYear(curTime);
    int nHs = int(dtCh / dtHs);

    /// initialize the transferred values, NO NEED to create and release in each timestep. lj
    map<int, float *> transferValuesMap;  // key is subbasinID, value is transferred values
    for (int i = 0; i < nSubbasins; i++) {
        float *tfvalues = nullptr;
        Initialize1DArray(tranferCount, tfvalues, NODATA_VALUE);
        transferValuesMap.insert(make_pair(pTasks[i], tfvalues));
    }

    for (; curTime <= dc->getSettingInput()->getEndTime(); curTime += dtCh) {
        if (slaveRank == 0) StatusMessage(ConvertToString2(&curTime).c_str());
        int yearIdx = GetYear(curTime) - startYear;
        tTask1 = MPI_Wtime();
        set<int> doneIDs; // save subbasin IDs that have been executed
        // 1. do the jobs that does not depend on other subbasins
        // 1.1 the slope and channel routing of source subbasins without upstreams
        for (auto itSrc = sourceBasins.begin(); itSrc != sourceBasins.end(); itSrc++) {
            ModelMain *pSubbasin = modelList[*itSrc];
            for (int i = 0; i < nHs; i++) {
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
            }
            pSubbasin->StepChannel(curTime, yearIdx);
            pSubbasin->AppendOutputData(curTime);

            if (!includeChannel) { continue; }

            int subbasinID = pTasks[*itSrc];
            // if the downstream subbasin is in this process,
            // there is no need to transfer values to the master process
            if (downStreamIdSet.find(pDownStream[*itSrc]) != downStreamIdSet.end()) {
                pSubbasin->GetTransferredValue(transferValuesMap[subbasinID]);
                doneIDs.insert(subbasinID);
                continue;
            }
            pSubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
            cout << "slave rank: " << slaveRank << "(world rank: " << world_rank << "), subbasinID: " << subbasinID
                 << " tfValues: ";
            for (int itf = MSG_LEN; itf < buflen; itf++) {
                cout << buf[itf] << ", ";
            }
            cout << endl;
#endif
            // transfer the result to the master process
            buf[0] = 1.f;  // message type
            buf[1] = subbasinID;   // subbasin id
            buf[2] = (float) t;
            MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
#ifdef _DEBUG
            cout << "Slave rank: " << slaveRank << "(world rank: " << world_rank << "), subbasin ID: " << subbasinID
                 << ", hillslope process of source subbasins done" << endl;
#endif /* _DEBUG */
        }

        // 1.2 the hillslope processes of downstream subbasins
        if (!includeChannel) { continue; }
        set<int> toDoSet;
        for (auto itDown = downStreamSet.begin(); itDown != downStreamSet.end(); itDown++) {
            ModelMain *pSubbasin = modelList[*itDown];
            for (int i = 0; i < nHs; i++) {
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
            }
            toDoSet.insert(*itDown);
        }
        tTask2 = MPI_Wtime();
        tSlope += tTask2 - tTask1;
        // TODO, tSlospe currently also contains the time of channel processes of source subbasins

        tTask1 = MPI_Wtime();
#ifdef _DEBUG
        cout << "slave_rank: " << slaveRank << "(world rank: " << world_rank << ")  step 2" << endl;
#endif
        // 2. The channel routing of downStream subbasin will be calculated
        //      if its upstream subbasins are already calculated
        set<int> canDoSet;
        while (!toDoSet.empty()) {
            // find all subbasins that the channel routing can be done without asking the master process
            for (auto itTodo = toDoSet.begin(); itTodo != toDoSet.end();) {
                bool upFinished = true;
                for (int j = 0; j < pUpNums[*itTodo]; j++) {
                    int upId = pUpStream[(*itTodo) * MAX_UPSTREAM + j];
                    // if can not find upstreams, this subbasin can not be done
                    if (doneIDs.find(upId) == doneIDs.end()) {
                        upFinished = false;
                        break;
                    }
                }
                if (upFinished) {
                    canDoSet.insert(*itTodo);
                    toDoSet.erase(itTodo++);
                } else {
                    itTodo++;
                }
            }

#ifdef _DEBUG
            cout << "slave_rank " << slaveRank << "(world rank: " << world_rank << ") todo set (subbasinID): ";
            for (auto itTodo = toDoSet.begin(); itTodo != toDoSet.end(); itTodo++) {
                cout << pTasks[*itTodo] << ", ";
            }
            cout << endl;
#endif
            // if can not find subbasins to calculate according to local information,
            // ask the master process if there are new upstream subbasins calculated
            if (canDoSet.empty()) {
                buf[0] = 2.f;
                buf[1] = groupId;
                buf[2] = world_rank;

                MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                MPI_Wait(&request, &status);

                int msgLen;
                MPI_Irecv(&msgLen, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                MPI_Wait(&request, &status);

                float *pData = new float[msgLen];
                MPI_Irecv(pData, msgLen, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                MPI_Wait(&request, &status);
#ifdef _DEBUG
                cout << "recv count of subbasin data: " << msgLen / (tranferCount + 1) << ", subbasinIDs: ";
                for (int ri = 0; ri < msgLen / (tranferCount + 1); ri++) {
                    cout << int(pData[ri * (tranferCount + 1)]) << ", ";
                }
                cout << endl;
#endif
                for (int ri = 0; ri < msgLen / (tranferCount + 1); ri++) {
                    int recv_subid = int(pData[ri * (tranferCount + 1)]);
                    if (transferValuesMap.find(recv_subid) == transferValuesMap.end()) {
                        float *tfvalues = nullptr;
                        Initialize1DArray(tranferCount, tfvalues, NODATA_VALUE);
                        transferValuesMap.insert(make_pair(recv_subid, tfvalues));
                    }
                    for (int vi = 0; vi < tranferCount; vi++) {
                        transferValuesMap[recv_subid][vi] = pData[ri * (tranferCount + 1) + vi + 1];
                    }
                    doneIDs.insert(recv_subid);
                }
                Release1DArray(pData);
            } else {  // canDoSet is not empty!
                // sort according to down-up layering from outlet
                vector<int> vec;
                set<int>::iterator itTmp, itMin;
                while (!canDoSet.empty()) {
                    itMin = canDoSet.begin();
                    for (itTmp = canDoSet.begin(); itTmp != canDoSet.end(); itTmp++) {
                        if (pDownupOrd[*itTmp] < pDownupOrd[*itMin]) {
                            itMin = itTmp;
                        }
                    }
                    vec.push_back(*itMin);
                    canDoSet.erase(itMin);
                }

                for (auto itCando = vec.begin(); itCando != vec.end(); itCando++) {
                    ModelMain *pSubbasin = modelList[*itCando];
                    float overFlowIn = 0.f;
                    for (int j = 0; j < pUpNums[*itCando]; ++j) {
                        int upId = pUpStream[(*itCando) * MAX_UPSTREAM + j];
                        pSubbasin->SetTransferredValue(upId, transferValuesMap[upId]);
                    }
                    pSubbasin->StepChannel(curTime, yearIdx);
                    pSubbasin->AppendOutputData(curTime);

                    if (downStreamIdSet.find(pDownStream[*itCando]) != downStreamIdSet.end()) {
                        pSubbasin->GetTransferredValue(transferValuesMap[pTasks[*itCando]]);
                        doneIDs.insert(pTasks[*itCando]);
                        continue;
                    }
                    // transfer the result to the master process
                    buf[0] = 1.f;  // message type
                    buf[1] = pTasks[*itCando];   // subbasin id
                    buf[2] = (float) t;
                    pSubbasin->GetTransferredValue(&buf[MSG_LEN]);
                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                }
            }
        }
        tTask2 = MPI_Wtime();
        tChannel += tTask2 - tTask1;

        MPI_Barrier(slaveComm);
        if (slaveRank == 0) {
            buf[0] = 0.f;
            MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
        }
        MPI_Barrier(slaveComm);
    }

    t2 = MPI_Wtime();
    t = t2 - t1;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    // TODO, Gather and scatter the computing time of each modules from all slave processors
    if (slaveRank == 0) {
        double computingTime = Max(tReceive, nSlaves);
#ifdef _DEBUG
        cout << "[DEBUG][TIMESPAN][COMPUTING]\tnprocs: " << numprocs - 1 << ", Max: " << computingTime
             << ", Total: " << Sum(nSlaves, tReceive) << "\n";
#endif
        cout << "[TIMESPAN][COMPUTING]\tALL\t" << fixed << setprecision(3) << computingTime << endl;
    }
    /***************  Outputs and combination  ***************/
    t1 = MPI_Wtime();
    for (int i = 0; i < nSubbasins; i++) {
        modelList[i]->Output();
    }
    t2 = MPI_Wtime();
    t = t2 - t1;

    /*** Combine raster outputs serially by one processor. ***/
    /// The operation could be considered as post-process,
    ///   therefore, the time-consuming is not included. 
    if (slaveRank == 0) {
        MongoGridFS *gfs = new MongoGridFS(mongoClient->getGridFS(input_args->m_model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput *outputs = dataCenterList[0]->getSettingOutput();
        for (auto it = outputs->m_printInfos.begin(); it != outputs->m_printInfos.end(); it++) {
            for (auto itemIt = (*it)->m_PrintItems.begin(); itemIt != (*it)->m_PrintItems.end(); itemIt++) {
                PrintInfoItem *item = *itemIt;
                StatusMessage(("Combining raster: " + item->Corename).c_str());
                if (item->m_nLayers >= 1) {
                    CombineRasterResultsMongo(gfs, item->Corename, dataCenterList[0]->m_nSubbasins,
                                              dataCenterList[0]->getOutputScenePath());
                }
            }
        }
        // clean up
        delete gfs;
    }
    /*** End of Combine raster outputs. ***/

    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0) {
        cout << "[TIMESPAN][IO]\tOUTPUT\t" << fixed << setprecision(3) << (t) << endl;
        cout << "[TIMESPAN][IO]\tALL\t" << fixed << setprecision(3) << (t + loadTime) << endl;
    }

    double tEnd = MPI_Wtime();
    t = tEnd - tStart;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0) {
        double allTime = Max(tReceive, nSlaves);
        cout << "[TIMESPAN][CALCULATION]\tALL\t" << allTime << "\n";
    }
    MPI_Barrier(slaveComm);

    // tell the master process to exit
    if (slaveRank == 0) {
        buf[0] = 9.f;
        MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
        MPI_Wait(&request, &status);
    }

    // clean up
    for (auto it = transferValuesMap.begin(); it != transferValuesMap.end();) {
        if (it->second != nullptr) {
            Release1DArray(it->second);
            it->second = nullptr;
        }
        transferValuesMap.erase(it++);
    }
    for (auto it = modelList.begin(); it != modelList.end();) {
        if (*it != nullptr) {
            delete *it;
        }
        it = modelList.erase(it);
    }
    for (auto it = dataCenterList.begin(); it != dataCenterList.end();) {
        if (*it != nullptr) {
            delete *it;
        }
        it = dataCenterList.erase(it);
    }
    delete moduleFactory;
    delete mongoClient;

    Release1DArray(buf);
    Release1DArray(tReceive);

    Release1DArray(pTasks);
    Release1DArray(pUpdownOrd);
    Release1DArray(pDownupOrd);
    Release1DArray(pDownStream);
    Release1DArray(pUpNums);
    Release1DArray(pUpStream);
}
