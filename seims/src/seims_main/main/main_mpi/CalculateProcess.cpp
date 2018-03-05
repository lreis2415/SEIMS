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
    double ioTime = 0;
    if (slaveRank == 0) {
        ioTime = Max(tReceive, nSlaves);
        cout << "[DEBUG]\tTime of reading data -- Max:" << ioTime << ", Total:" << Sum(nSlaves, tReceive) << "\n";
        cout << "[DEBUG][TIMESPAN][IO]" << ioTime << endl;
    }
    t1 = MPI_Wtime();

    // classification according to the Layering method, i.e., up-down and down-up orders.
    LayeringMethod lyrmtd = input_args->m_layer_mtd;
    vector<int> sourceBasins;
    // used to find if the downstream subbasin of a finished subbsin is in the same process,
    // if so, the MPI send operation is not necessary.
    // the set container is more efficient for the 'find' operation
    set<int> downStreamSet;  ///< not source subbasins
    set<int> downStreamIdSet;
    bool includeChannel = false;
    if (modelList[0]->IncludeChannelProcesses()) {
        includeChannel = true;
        for (int i = 0; i < nSubbasins; i++) {
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
    cout << "Slave rank: " << slaveRank << ", Source subbasins index: ";
    for (auto it = sourceBasins.begin(); it != sourceBasins.end(); it++) {
        cout << *it << ", ";
    }
    cout << endl;
#endif /* _DEBUG */
    double tTask1, tTask2;
    double tSlope = 0.0;
    double tChannel = 0.0;;
    //float buf[MSG_LEN];
    float *buf = nullptr;
    Initialize1DArray(MSG_LEN, buf, NODATA_VALUE);
    // time loop
    DataCenterMongoDB *dc = dataCenterList[0];
    ModelMain *p = modelList[0];
    time_t dtHs = dc->getSettingInput()->getDtHillslope();
    time_t dtCh = dc->getSettingInput()->getDtChannel();

    time_t curTime = dc->getSettingInput()->getStartTime();
    int startYear = GetYear(curTime);
    int nHs = int(dtCh / dtHs);
    for (; curTime <= dc->getSettingInput()->getEndTime(); curTime += dtCh) {
        if (slaveRank == 0) StatusMessage(ConvertToString2(&curTime).c_str());
        int yearIdx = GetYear(curTime) - startYear;

        map<int, float> qMap; //used to contain the flowout of each subbasin

        tTask1 = MPI_Wtime();
        // 1. do the jobs that does not depend on other subbasins
        // 1.1 the slope and channel routing of source subbasins without upstreams
        for (auto it = sourceBasins.begin(); it != sourceBasins.end(); it++) {
            ModelMain *pSubbasin = modelList[*it];
            for (int i = 0; i < nHs; i++) {
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
            }
            pSubbasin->StepChannel(curTime, yearIdx);
            pSubbasin->AppendOutputData(curTime);

            if (!includeChannel) { continue; }

            float qOutlet = pSubbasin->GetQOutlet();
            int subbasinID = pTasks[*it];

            //if(world_rank == R)
            //	cout << world_rank << ":" << subbasinID << " " << qOutlet << endl;

            // if the downstream subbasin is in the s process,
            // there is no need to transfer outflow to the master process
            if (downStreamIdSet.find(pDownStream[*it]) != downStreamIdSet.end()) {
                qMap[subbasinID] = qOutlet;
                //if(world_rank == R)
                //    cout << "qMap: " << qMap[subbasinID] << endl;
                continue;
            }
            //if(world_rank == R)
            //    cout << world_rank << ": pass" << pTasks[sourceBasins[j]] << "\n";

            // transfer the result to the master process
            buf[0] = 1.f;  // message type
            buf[1] = subbasinID;   // subbasin id
            buf[2] = qOutlet;// flow out of subbasin
            buf[3] = t;
            MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
#ifdef _DEBUG
            cout << "Slave rank: " << slaveRank << ", subbasin ID: " << pTasks[*it]
                 << ", hillslope process of source subbasins done" << endl;
#endif /* _DEBUG */
        }

        // 1.2 the slope routing of downstream subbasins
        if (!includeChannel) { continue; }
        for (auto it = downStreamSet.begin(); it != downStreamSet.end(); it++) {
            ModelMain *pSubbasin = modelList[*it];
            for (int i = 0; i < nHs; i++) {
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
            }
        }
        tTask2 = MPI_Wtime();
        tSlope = tSlope + tTask2 - tTask1;

        tTask1 = MPI_Wtime();
        cout << "world_rank: " << world_rank << "  step 2" << endl;
        // 2. the channel routing of  downStream subbasins are calculated
        // if their upstream subbasins are already calculated
        set<int> toDoSet, canDoSet;
        //cout << "test world_rank: " << world_rank << " >> ";
        for (auto it = downStreamSet.begin(); it != downStreamSet.end(); it++) {
            toDoSet.insert(*it);
        }
        while (!toDoSet.empty()) {
            // find all subbasins that the channel routing can be done without asking the master process
            for (auto it = toDoSet.begin(); it != toDoSet.end();) {
                bool upFinished = true;
                for (int j = 0; j < pUpNums[*it]; j++) {
                    int upId = pUpStream[(*it) * MAX_UPSTREAM + j];
                    // if can not find upstreams, this subbasin can not be done
                    if (qMap.find(upId) == qMap.end()) {
                        upFinished = false;
                        break;
                    }
                }
                if (upFinished) {
                    canDoSet.insert(*it);
                    toDoSet.erase(it++);
                } else {
                    it++;
                }
            }

#ifdef _DEBUG
            cout << "world_rank " << world_rank << "  todo set: ";
            for (auto it = toDoSet.begin(); it != toDoSet.end(); it++) {
                cout << pTasks[*it] << " ";
            }
            cout << endl;
#endif
            // if can not find subbasins to calculate according to local information,
            // ask the master process if there are new upstream subbasins calculated
            if (canDoSet.empty()) {
                buf[0] = 2.f;
                buf[1] = groupId;
                buf[2] = world_rank;

                MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                MPI_Wait(&request, &status);

                int msgLen;
                MPI_Irecv(&msgLen, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                MPI_Wait(&request, &status);

                float *pData = new float[msgLen];
                MPI_Irecv(pData, msgLen, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                MPI_Wait(&request, &status);


                //cout << "recv world_rank" << world_rank << "  num:" << msgLen/2 << " data:";
                for (int j = 0; j < msgLen; j += 2) {
                    //cout << pData[j] << " ";
                    qMap[(int) pData[j]] = pData[j + 1];
                }
                //cout << endl;
                //cout << "world_rank" << world_rank << " qMap: ";
                //for (map<int,float>::iterator it = qMap.begin(); it != qMap.end(); it++)
                //{
                //	cout << it->first << " ";
                //}
                //cout << endl;

                delete pData;
            } else {
                //cout << "world_rank:" << world_rank << endl;
                // sort according to the distance to outlet descendent
                vector<int> vec;
                set<int>::iterator it, itMax;
                while (!canDoSet.empty()) {
                    itMax = canDoSet.begin();
                    for (it = canDoSet.begin(); it != canDoSet.end(); ++it) {
                        if (pDownupOrd[*it] > pDownupOrd[*itMax]) {
                            itMax = it;
                        }
                    }
                    vec.push_back(*itMax);
                    canDoSet.erase(itMax);
                }

                for (auto it = vec.begin(); it != vec.end(); it++) {
                    int index = *it;
                    ModelMain *pSubbasin = modelList[index];

                    //cout << "index:" << index << endl;
                    float overFlowIn = 0.f;
                    for (int j = 0; j < pUpNums[index]; ++j) {
                        int upId = pUpStream[index * MAX_UPSTREAM + j];
                        overFlowIn += qMap[upId];
                    }
                    pSubbasin->SetChannelFlowIn(overFlowIn);
                    pSubbasin->StepChannel(curTime, yearIdx);
                    pSubbasin->AppendOutputData(curTime);

                    float qOutlet = pSubbasin->GetQOutlet();

                    if (downStreamIdSet.find(pDownStream[index]) != downStreamIdSet.end()) {
                        qMap[pTasks[index]] = qOutlet;
                        continue;
                    }
                    // transfer the result to the master process
                    buf[0] = 1.f;  // message type
                    buf[1] = pTasks[index];   // subbasin id
                    buf[2] = qOutlet;// flow out of subbasin
                    buf[3] = t;
                    MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                }

                //cout << "world_rank" << world_rank << " qMap after execute: ";
                //for(auto it = qMap.begin(); it != qMap.end(); ++it)
                //	cout << it->first << " ";
                //cout << endl;
            }
        }
        tTask2 = MPI_Wtime();
        tChannel = tChannel + tTask2 - tTask1;

        MPI_Barrier(slaveComm);
        if (slaveRank == 0) {
            buf[0] = 0.f;
            MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
        }
        MPI_Barrier(slaveComm);
    }

    t2 = MPI_Wtime();
    t = t2 - t1;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0) {
        double computingTime = Max(tReceive, nSlaves);
        //cout << "[DEBUG]\tnprocs: " << numprocs - 1 << ", Max: " << computingTime
        //     << ", Total: " << Sum(nSlaves, tReceive) << "\n";
        cout << "[DEBUG][TIMESPAN][COMPUTING]" << computingTime << "\n";
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
        MongoGridFS* gfs = new MongoGridFS(mongoClient->getGridFS(input_args->m_model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput* outputs = dataCenterList[0]->getSettingOutput();
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

    double tEnd = MPI_Wtime();
    t = tEnd - tStart;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0) {
        double allTime = Max(tReceive, nSlaves);
        cout << "[DEBUG][TIMESPAN][TOTAL]" << allTime << "\n";
    }
    MPI_Barrier(slaveComm);

    // tell the master process to exit
    if (slaveRank == 0) {
        buf[0] = 9.f;
        MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
        MPI_Wait(&request, &status);
    }

    // clean up
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
