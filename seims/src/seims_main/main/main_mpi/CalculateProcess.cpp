#include "parallel.h"
#include "CombineRaster.h"

void CalculateProcess(int worldRank, int numprocs, int nSlaves, MPI_Comm slaveComm, InputArgs* inputArgs) {
    double tStart = MPI_Wtime();
    int slaveRank;
    MPI_Comm_rank(slaveComm, &slaveRank);
    StatusMessage(("Enter computing process, worldRank: " + ValueToString(worldRank) +
                      ", slave_rank: " + ValueToString(slaveRank)).c_str());

    MPI_Request request;
    MPI_Status status;

    // receive task information from master process, and scatter to all slave processors
    int* pTaskAll = nullptr;
    int* pUpdownOrdAll = nullptr;
    int* pDownupOrdAll = nullptr;
    int* pDownStreamAll = nullptr;
    int* pUpNumsAll = nullptr;
    int* pUpStreamAll = nullptr;
    int* pGroupId = nullptr;
    int maxTaskLen;

    if (worldRank == SLAVE0_RANK) {
        // in this block, the communicator is MCW
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
    /// Subscript of the following variables is the index of subbasins in current slave rank,
    /// NOT the subbasin ID. Be careful when used.
    int* pTasks = new int[maxTaskLen]; ///< subbasin IDs of each task for current rank
    int* pUpdownOrd = new int[maxTaskLen]; ///< Up-down stream order
    int* pDownupOrd = new int[maxTaskLen]; ///< Down-up stream order
    int* pDownStream = new int[maxTaskLen]; ///< Downstream subbasin ID
    int* pUpNums = new int[maxTaskLen]; ///< Upstream subbasins number
    int* pUpStream = new int[maxTaskLen * MAX_UPSTREAM]; ///< Upstream subbasin IDs

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
    cout << "    SubbasinID, UpDownOrder, DownUpOrder, DownStream, UpStreams" << endl;
    for(int i = 0; i < maxTaskLen; i++) {
        if(pTasks[i] < 0) continue;
        cout << "    " << pTasks[i] << ", " << pUpdownOrd[i] << ", " << pDownupOrd[i] << ", " << pDownStream[i] <<
                ", ups:";
        for(int j = 0; j < pUpNums[i]; j++) {
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

    int nSubbasins = 0; // subbasin number of current worldRank
    for (int i = 0; i < maxTaskLen; i++) {
        if (pTasks[i] > 0) nSubbasins++;
    }
    if (nSubbasins == 0) {
        cout << "No task for Rank " << worldRank << endl;
        MPI_Abort(MCW, 1);
    }
    double t1, t2;
    double t;
    t1 = MPI_Wtime();

    ////////////////////////////////////////////////////////////////////
    /// Get module path
    string modulePath = GetAppPath();
    // setup model runs for subbasins
    MongoClient* mongoClient = MongoClient::Init(inputArgs->m_host_ip, inputArgs->m_port);
    if (nullptr == mongoClient) {
        throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
    }
    /// Create module factory
    ModuleFactory* moduleFactory = ModuleFactory::Init(modulePath, inputArgs);
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
        DataCenterMongoDB* dataCenter = new DataCenterMongoDB(inputArgs, mongoClient, moduleFactory, pTasks[i]);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model = new ModelMain(dataCenter, moduleFactory);

        dataCenterList.push_back(dataCenter);
        modelList.push_back(model);
    }
    StatusMessage(("Slave rank " + ValueToString(slaveRank) + " model construct done!").c_str());
    // print IO time
    t2 = MPI_Wtime();
    t = t2 - t1;
    double* tReceive = new double[nSlaves];
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

    int maxLyrNum = 1;
    /*! Source subbasins in each layer in current slave rank
     * Key: Layering number
     * Value: Source subbasin indexes
     */
    map<int, vector<int> > srcSubbsnLayers;
    /*! Non source subbasins in each layer in current slave rank
     * Key: Layering number
     * Value: Non source subbasin indexes
     */
    map<int, vector<int> > nonSrcSubbsnLayers;
    // Used to find if the downstream subbasin of a finished subbsin is in the same process,
    //   if so, the MPI send operation is not necessary.
    //   the `set` container is more efficient for the 'find' operation
    set<int> downStreamSet; // Indexes of not-first layer subbasins
    set<int> downStreamIdSet; // Subbasin IDs of not-first layer subbasins
    bool includeChannel = false;
    if (modelList[0]->IncludeChannelProcesses()) {
        includeChannel = true;
        for (int i = 0; i < nSubbasins; i++) {
            // classification according to the Layering method, i.e., up-down and down-up orders.
            int streamOrder = inputArgs->m_layer_mtd == UP_DOWN ? pUpdownOrd[i] : pDownupOrd[i];
            if (streamOrder > maxLyrNum) { maxLyrNum = streamOrder; }
            if (streamOrder > 1) {
                // all possible downstream subbasins (i.e., layer >= 2)
                downStreamSet.insert(i); // index of the array
                downStreamIdSet.insert(pTasks[i]); // Subbasin ID
            }
            if (pUpNums[i] == 0) {
                // source subbasins of each layer
                if (srcSubbsnLayers.find(streamOrder) == srcSubbsnLayers.end()) {
                    srcSubbsnLayers.insert(make_pair(streamOrder, vector<int>()));
                }
                srcSubbsnLayers[streamOrder].push_back(i);
            } else {
                if (nonSrcSubbsnLayers.find(streamOrder) == nonSrcSubbsnLayers.end()) {
                    nonSrcSubbsnLayers.insert(make_pair(streamOrder, vector<int>()));
                }
                nonSrcSubbsnLayers[streamOrder].push_back(i);
            }
        }
    } else {
        // No channel processes are simulated, all subbasins will be regarded as source subbasins
        srcSubbsnLayers.insert(make_pair(1, vector<int>()));
        for (int i = 0; i < nSubbasins; i++) {
            srcSubbsnLayers[1].push_back(i);
        }
    }
#ifdef _DEBUG
    cout << "Slave rank: " << slaveRank << "(world rank: " << worldRank << "), Source subbasins of layer 1: ";
    for(auto it = srcSubbsnLayers[1].begin(); it != srcSubbsnLayers[1].end(); ++it) {
        cout << pTasks[*it] << ", ";
    }
    cout << endl;
#endif /* _DEBUG */

    double tTask1, tTask2;
    double tSlope = 0.0;
    double tChannel = 0.0;;
    // Create buffer for passing values across subbasins
    float* buf = nullptr;
    int buflen = MSG_LEN + tranferCount;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    // Get timesteps
    DataCenterMongoDB* dc = dataCenterList[0];
    time_t dtHs = dc->getSettingInput()->getDtHillslope();
    time_t dtCh = dc->getSettingInput()->getDtChannel();
    time_t curTime = dc->getSettingInput()->getStartTime();
    int startYear = GetYear(curTime);
    int nHs = int(dtCh / dtHs);

    // initialize the transferred values, NO NEED to create and release in each timestep.
    map<int, float *> transferValuesMap; // key is subbasinID, value is transferred values
    for (int i = 0; i < nSubbasins; i++) {
        float* tfvalues = nullptr;
        Initialize1DArray(tranferCount, tfvalues, NODATA_VALUE);
        transferValuesMap.insert(make_pair(pTasks[i], tfvalues));
    }

    // Simulation loop
    for (; curTime <= dc->getSettingInput()->getEndTime(); curTime += dtCh) {
        if (slaveRank == 0) StatusMessage(ConvertToString2(&curTime).c_str());
        int yearIdx = GetYear(curTime) - startYear;
        tTask1 = MPI_Wtime();
        set<int> doneIDs; // save subbasin IDs that have been executed
        // Execute by layering orders
        for (int iLyr = 1; iLyr <= maxLyrNum; iLyr++) {
#ifdef _DEBUG
            cout << "slave_rank: " << slaveRank << "(world rank: " << worldRank << "), Layer " << iLyr << endl;
#endif
            // 1. Execute subbasins that does not depend on others
            if (srcSubbsnLayers.find(iLyr) != srcSubbsnLayers.end()) {
                for (auto itSrc = srcSubbsnLayers[iLyr].begin(); itSrc != srcSubbsnLayers[iLyr].end(); ++itSrc) {
#ifdef _DEBUG
                    cout << "    layer: " << iLyr << ", source subbasin: " << pTasks[*itSrc] << endl;
#endif
                    // 1.1 Execute hillslope and channel processes, just like the serial version
                    ModelMain* pSubbasin = modelList[*itSrc];
                    for (int i = 0; i < nHs; i++) {
                        pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
                    }
                    pSubbasin->StepChannel(curTime, yearIdx);
                    pSubbasin->AppendOutputData(curTime);

                    if (!includeChannel) { continue; }

                    // 1.2 If the downstream subbasin is in this process,
                    //     there is no need to transfer values to the master process
                    int subbasinID = pTasks[*itSrc];
                    if (downStreamIdSet.find(pDownStream[*itSrc]) != downStreamIdSet.end()) {
                        pSubbasin->GetTransferredValue(transferValuesMap[subbasinID]);
                        doneIDs.insert(subbasinID);
                        continue;
                    }
                    // 1.3 Otherwise, the transferred values of current subbasin should be sent to master rank
                    pSubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                    cout << "slave rank: " << slaveRank << "(world rank: " << worldRank << "), send subbasinID: " <<
                            subbasinID << " tfValues: ";
                    for(int itf = MSG_LEN; itf < buflen; itf++) {
                        cout << buf[itf] << ", ";
                    }
                    cout << endl;
#endif
                    buf[0] = 1.f; // message type: Send subbasin data to master rank
                    buf[1] = subbasinID; // subbasin ID
                    buf[2] = float(t);
                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                } /* srcSubbsnLayers[iLyr] loop */
            } /* If srcSubbsnLayers has iLyr */

            // 2. Execute subbasins that depend on others
            if (nonSrcSubbsnLayers.find(iLyr) == nonSrcSubbsnLayers.end()) { continue; }
            // some layers may absent in current slave rank
            if (!includeChannel) { continue; }

            // 2.1 Execute hillslope processes
            set<int> toDoSet; // To be executed subbasin indexes of current layer
            for (auto itDown = nonSrcSubbsnLayers[iLyr].begin(); itDown != nonSrcSubbsnLayers[iLyr].end(); ++itDown) {
#ifdef _DEBUG
                cout << "    layer: " << iLyr << ", non-source subbasin(hillslope): " << pTasks[*itDown] << endl;
#endif
                ModelMain* pSubbasin = modelList[*itDown];
                for (int i = 0; i < nHs; i++) {
                    pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
                }
                toDoSet.insert(*itDown);
            }
            tTask2 = MPI_Wtime();
            tSlope += tTask2 - tTask1;
            // TODO, tSlospe currently also contains the time of channel processes of source subbasins
            tTask1 = MPI_Wtime();

            // 2.2 The channel routing of downStream subbasin will be calculated
            //     if its upstream subbasins are already calculated
            set<int> canDoSet;
            while (!toDoSet.empty()) {
                // 2.2.1 Find all subbasins that the channel routing can be done
                //       without asking the master process for data
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
                        ++itTodo;
                    }
                }

#ifdef _DEBUG
                cout << "slave_rank " << slaveRank << "(world rank: " << worldRank << ") todo set (subbasinID): ";
                for(auto itTodo = toDoSet.begin(); itTodo != toDoSet.end(); ++itTodo) {
                    cout << pTasks[*itTodo] << ", ";
                }
                cout << ", cando set (subbasinID): ";
                for(auto itCando = canDoSet.begin(); itCando != canDoSet.end(); ++itCando) {
                    cout << pTasks[*itCando] << ", ";
                }
                cout << endl;
#endif
                // 2.2.2 If can not find subbasins to calculate according to local information,
                //       ask the master process if there are new upstream subbasins calculated
                if (canDoSet.empty()) {
                    buf[0] = 2.f; // Ask for the already calculated subbasins from other slave ranks
                    buf[1] = groupId;
                    buf[2] = worldRank;

                    MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                    // Wait and receive data from master rank
                    int msgLen;
                    MPI_Irecv(&msgLen, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    MPI_Wait(&request, &status);

                    float* pData = new float[msgLen];
                    MPI_Irecv(pData, msgLen, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
#ifdef _DEBUG
                    cout << "recv count of subbasin data: " << msgLen / (tranferCount + 1) << ", subbasinIDs: ";
                    for(int ri = 0; ri < msgLen / (tranferCount + 1); ri++) {
                        cout << int(pData[ri * (tranferCount + 1)]) << ", ";
                    }
                    cout << endl;
#endif
                    for (int ri = 0; ri < msgLen / (tranferCount + 1); ri++) {
                        int recv_subid = int(pData[ri * (tranferCount + 1)]);
                        if (transferValuesMap.find(recv_subid) == transferValuesMap.end()) {
                            float* tfvalues = nullptr;
                            Initialize1DArray(tranferCount, tfvalues, NODATA_VALUE);
                            transferValuesMap.insert(make_pair(recv_subid, tfvalues));
                        }
                        for (int vi = 0; vi < tranferCount; vi++) {
                            transferValuesMap[recv_subid][vi] = pData[ri * (tranferCount + 1) + vi + 1];
                        }
                        doneIDs.insert(recv_subid);
                    }
                    Release1DArray(pData);
                } else {
                    // 2.2.3 The required subbasin data has been satisfied,
                    //       execute subbasin in canDoSet, and send data to master rank if necessary
                    for (auto itCando = canDoSet.begin(); itCando != canDoSet.end();) {
#ifdef _DEBUG
                        cout << "    layer: " << iLyr << ", non-source subbasin(channel): " << pTasks[*itCando] << endl;
#endif
                        ModelMain* pSubbasin = modelList[*itCando];
                        for (int j = 0; j < pUpNums[*itCando]; j++) {
                            int upId = pUpStream[(*itCando) * MAX_UPSTREAM + j];
                            pSubbasin->SetTransferredValue(upId, transferValuesMap[upId]);
                        }
                        pSubbasin->StepChannel(curTime, yearIdx);
                        pSubbasin->AppendOutputData(curTime);

                        if (downStreamIdSet.find(pDownStream[*itCando]) != downStreamIdSet.end()) {
                            pSubbasin->GetTransferredValue(transferValuesMap[pTasks[*itCando]]);
                            doneIDs.insert(pTasks[*itCando]);
                            canDoSet.erase(itCando++); // remove current subbasin from canDoSet
                            continue;
                        }
                        // transfer the result to the master process
                        buf[0] = 1.f; // message type: Send subbasin data to master rank
                        buf[1] = pTasks[*itCando]; // subbasin ID
                        buf[2] = float(t);
                        pSubbasin->GetTransferredValue(&buf[MSG_LEN]);
#ifdef _DEBUG
                        cout << "slave rank: " << slaveRank << "(world rank: " << worldRank << "), send subbasinID: "
                             << pTasks[*itCando] << ", tfValues: ";
                        for(int itf = MSG_LEN; itf < buflen; itf++) {
                            cout << buf[itf] << ", ";
                        }
                        cout << endl;
#endif
                        MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
                        MPI_Wait(&request, &status);
                        canDoSet.erase(itCando++);
                    } /* canDoSet loop of current layer */
                } /* canDoSet is not empty */
            } /* while toDoSet is not empty */

            tTask2 = MPI_Wtime();
            tChannel += tTask2 - tTask1;
        } /* subbasin layers loop */

        MPI_Barrier(slaveComm);
        if (slaveRank == 0) {
            buf[0] = 0.f; // signal for new timestep
            MPI_Isend(buf, buflen, MPI_FLOAT, MASTER_RANK, WORK_TAG, MCW, &request);
            MPI_Wait(&request, &status);
        }
    } /* timestep loop */

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
        MongoGridFS* gfs = new MongoGridFS(mongoClient->getGridFS(inputArgs->m_model_name, DB_TAB_OUT_SPATIAL));
        SettingsOutput* outputs = dataCenterList[0]->getSettingOutput();
        for (auto it = outputs->m_printInfos.begin(); it != outputs->m_printInfos.end(); ++it) {
            for (auto itemIt = (*it)->m_PrintItems.begin(); itemIt != (*it)->m_PrintItems.end(); ++itemIt) {
                PrintInfoItem* item = *itemIt;
                if (item->m_nLayers >= 1) {
                    // Only need to handle raster data
                    StatusMessage(("Combining raster: " + item->Corename).c_str());
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
