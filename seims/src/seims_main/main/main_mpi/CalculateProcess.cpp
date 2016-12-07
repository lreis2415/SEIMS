#include "parallel.h"
#include "mpi.h"
#include "util.h"

#include <sstream>
#include <iostream>
#include "PrintInfo.h"
#include "ModelMain.h"
#include "invoke.h"
#include "ModuleFactory.h"
#include "ClimateParams.h"
#include "clsRasterData.cpp"
//#include "SimulationModule.h"
#include "mongoc.h"
// #include "mongo.h"

void CalculateProcess(int rank, int numprocs, int nSlaves, MPI_Comm slaveComm,
                      string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
                      int nThreads, LayeringMethod layeringMethod)
{
    double tStart = MPI_Wtime();

    MPI_Request request;
    MPI_Status status;

    // receive task information from master process
    int *pTaskAll = NULL;
    int *pRankAll = NULL;
    int *pDisAll = NULL;
    int *pDownStreamAll = NULL;
    int *pUpNumsAll = NULL;
    int *pUpStreamAll = NULL;
    int *pGroupId = NULL;
    int maxTaskLen;
    //MPI_Barrier(MPI_COMM_WORLD);

    if (rank == SLAVE0_RANK)
    {
        //cout << "Number of threads: " << nThreads << endl;

        int nTaskAll;
        MPI_Status status;
        MPI_Recv(&nTaskAll, 1, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        pTaskAll = new int[nTaskAll];
        pRankAll = new int[nTaskAll];
        pDisAll = new int[nTaskAll];
        pDownStreamAll = new int[nTaskAll];
        pUpNumsAll = new int[nTaskAll];
        pUpStreamAll = new int[nTaskAll * MAX_UPSTREAM];
        pGroupId = new int[nSlaves];

        MPI_Recv(pGroupId, nSlaves, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pTaskAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pRankAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pDisAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pDownStreamAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pUpNumsAll, nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(pUpStreamAll, MAX_UPSTREAM * nTaskAll, MPI_INT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &status);

        maxTaskLen = nTaskAll / nSlaves;
    }

    //if (rank == 1)
    //	cout << "RANK:" << rank << "maxTaskLen:" << maxTaskLen << endl;
    MPI_Bcast(&maxTaskLen, 1, MPI_INT, 0, slaveComm);

    int groupId;
    int *pTasks = new int[maxTaskLen];
    int *pRanks = new int[maxTaskLen];
    int *pDis = new int[maxTaskLen];
    int *pDownStream = new int[maxTaskLen];
    int *pUpNums = new int[maxTaskLen];
    int *pUpStream = new int[maxTaskLen * MAX_UPSTREAM];

    MPI_Scatter(pGroupId, 1, MPI_INT, &groupId, 1, MPI_INT, 0, slaveComm);
    MPI_Scatter(pTaskAll, maxTaskLen, MPI_INT, pTasks, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pRankAll, maxTaskLen, MPI_INT, pRanks, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pDisAll, maxTaskLen, MPI_INT, pDis, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pDownStreamAll, maxTaskLen, MPI_INT, pDownStream, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pUpNumsAll, maxTaskLen, MPI_INT, pUpNums, maxTaskLen, MPI_INT, 0, slaveComm);
    MPI_Scatter(pUpStreamAll, maxTaskLen * MAX_UPSTREAM, MPI_INT, pUpStream, maxTaskLen * MAX_UPSTREAM, MPI_INT, 0,
                slaveComm);

    int slaveRank;
    MPI_Comm_rank(slaveComm, &slaveRank);

    //ostringstream oss;
    //oss << "Subbasins of slave process " << slaveRank << ":  \n";
    //for (int i = 0; i < maxTaskLen; i++)
    //{
    //if(pTasks[i] < 0) continue;
    //oss << pTasks[i] << " ";// <<  pRanks[i] << " " << pDownStream[i] << " ups:";
    //for(int j = 0; j < pUpNums[i]; j++)
    //{
    //    oss << pUpStream[MAX_UPSTREAM*i + j] << " ";
    //}
    //oss << "\n";
    //}
    //oss << "\n";
    //cout << oss.str();
    //cout << "rank " << rank << " received tasks.\n";
    delete[] pTaskAll;
    delete[] pRankAll;
    delete[] pDisAll;
    delete[] pDownStreamAll;
    delete[] pUpNumsAll;
    delete[] pUpStreamAll;
    // end of assign tasks
    //////////////////////////////////////////////////////////////////////////////////////

    int nSubbasins = 0;
    for (int i = 0; i < maxTaskLen; ++i)
    {
        if (pTasks[i] > 0)
            nSubbasins++;
    }
    double t1, t2;
    double t;
    t1 = MPI_Wtime();

    // setup models for subbasins
    vector<ModelMain *> modelList;
    modelList.reserve(nSubbasins);

    ////////////////////////////////////////////////////////////////////
    //cout << "rank " << rank << " check project.\n";
    checkProject(projectPath);

    //string dbName = "model_1";
    // mongo conn[1];
    //const char* host = "127.0.0.1";
    //int port = 27017;

    //int mongoStatus = mongo_connect(conn, host, port);
    //if (MONGO_OK != mongoStatus)
    //{
    //    cout << "can not connect to mongodb.\n";
    //    exit(-1);
    //}
    //checkDatabase(conn, string(dbName));

	mongoc_client_t *conn;
	if (!isIPAddress(host))
		throw ModelException("MainMongoDB", "Connect to MongoDB",
		"IP address: " + string(host) + "is invalid, Please check!\n");
	mongoc_init();
	mongoc_uri_t *uri = mongoc_uri_new_for_host_port(host, port);
	conn = mongoc_client_new_from_uri(uri);

    // ModuleFactory *factory = new ModuleFactory(projectPath + File_Config, modulePath, conn, string(dbName));
	int nSubbasin = 1;
	int scenarioID = 0;
	ModuleFactory *factory = new ModuleFactory(projectPath + File_Config, modulePath, conn, string(dbName), nSubbasin, layeringMethod, scenarioID);
    string db = dbName;
    string inputFile = projectPath + File_Input;
    for (int i = 0; i < nSubbasins; i++)
    {
        //cout << rank << " " <<  pTasks[i] << endl;
		SettingsInput *input = new SettingsInput(inputFile, conn, db, pTasks[i]);
        // ModelMain *p = new ModelMain(conn, db, projectPath, factory, pTasks[i], 0, layeringMethod);
		ModelMain *p = new ModelMain(conn, db, projectPath, input, factory, pTasks[i], scenarioID, nThreads, layeringMethod);
        //if(i == 0)
        //{
        //    SettingsInput *input = new SettingsInput(inputFile, conn, db, pTasks[i]);
        //    p->Init(input, nThreads);
        //}
        modelList.push_back(p);
    }
    //cout << rank << " after constructor\n";

    t2 = MPI_Wtime();
    t = t2 - t1;
    double *tReceive = new double[nSlaves];
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    double ioTime = 0;
    if (slaveRank == 0)
    {
        ioTime = Max(tReceive, nSlaves);
        //cout << "[DEBUG]\tTime of reading data -- Max:" << ioTime << "   Total:" << Sum(tReceive, nSlaves) << "\n";
		cout << "[DEBUG]\tTime of reading data -- Max:" << ioTime << "   Total:" << Sum(nSlaves, tReceive) << "\n";
        cout << "[DEBUG][TIMESPAN][IO]" << ioTime << endl;
    }
    t1 = MPI_Wtime();

    // classification according to the rank of subbasin
    vector<int> sourceBasins;
    set<int> downStreamSet, downStreamIdSet; // used to find if the downstream subbasin of a finished subbsin is in the same process,
    // if so, the MPI send operation is not necessary.
    // the set container is more efficient for the finding operation
    bool includeChannel = false;
    if (modelList[0]->IncludeChannelProcesses())
    {
        includeChannel = true;
        for (int i = 0; i < nSubbasins; i++)
        {
            if (pRanks[i] == 1)
                sourceBasins.push_back(i);
            else
            {
                downStreamSet.insert(i);//index of the array
                downStreamIdSet.insert(pTasks[i]);//id of subbasin
            }
        }
    }
        // if no channel processes are simulated
    else
    {
        for (int i = 0; i < nSubbasins; i++)
            sourceBasins.push_back(i);
    }

    double tTask1, tTask2;
    double tSlope = 0.0;
    double tChannel = 0.0;;
    float buf[MSG_LEN];
    // time loop
    ModelMain *p = modelList[0];
    int dtHs = p->getDtHillSlope();
    int dtCh = p->getDtChannel();

    int R = 1;
    //cout << p->getStartTime() << "\t" << p->getEndTime() << "\t" << dtCh << endl;
    utils util;
    //cout << "Whether include channel: " << includeChannel << endl;
	time_t curTime = p->getStartTime();
	int startYear = GetYear(curTime);
    for ( ; curTime <= p->getEndTime(); curTime += dtCh)
    {
		int yearIdx = GetYear(curTime) - startYear;
        int nHs = int(dtCh / dtHs);

        //if(slaveRank == 0)
        //	cout << util.ConvertToString2(&t) << endl;
        map<int, float> qMap; //used to contain the flowout of each subbasin

        tTask1 = MPI_Wtime();
        // 1. do the jobs that does not depend on other subbasins
        // 1.1 the slope and channel routing of source subbasins without upstreams
        //if(rank == R)
        //	cout << "RANK" << rank << "  size:" <<  sourceBasins.size() << "  " << includeChannel << "  " << nHs << endl;
        for (size_t j = 0; j < sourceBasins.size(); ++j)
        {
            int index = sourceBasins[j];
            ModelMain *pSubbasin = modelList[index];
            //if(!pSubbasin->IsInitialized())
            //{
            //cout << pTasks[index] << endl;
            //    SettingsInput *input = new SettingsInput(inputFile, conn, db, pTasks[index]);
            //    pSubbasin->Init(input, nThreads);
            //}

            //if (rank == R)
            //	cout << "RANK" << rank << ":" << pTasks[index] << " Before execution" << endl;
            //modelList[index]->Step(t);
            for (int i = 0; i < nHs; ++i)
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
            //if (rank == R)
            //	cout << "RANK" << rank << ":" << pTasks[index] << " End Hillslope execution" << endl;
            pSubbasin->StepChannel(curTime, yearIdx);
            //if (rank == R)
            //	cout << "RANK" << rank << ":" << pTasks[index] << " End source execution" << endl;

            pSubbasin->Output(curTime);
            //if (rank == R)
            //	cout << "RANK" << rank << ":" << pTasks[index] << " End output" << endl;
            if (includeChannel)
            {
                float qOutlet = pSubbasin->GetQOutlet();
                int subbasinID = pTasks[index];

                //if(rank == R)
                //	cout << rank << ":" << subbasinID << " " << qOutlet << endl;

                // if the downstream subbasin is in the s process,
                // there is no need to transfer outflow to the master process
                if (downStreamIdSet.find(pDownStream[index]) != downStreamIdSet.end())
                {
                    qMap[subbasinID] = qOutlet;
                    //if(rank == R)
                    //    cout << "qMap: " << qMap[subbasinID] << endl;
                    continue;
                }
                //if(rank == R)
                //    cout << rank << ": pass" << pTasks[sourceBasins[j]] << "\n";

                // transfer the result to the master process
                buf[0] = 1.f;  // message type
                buf[1] = subbasinID;   // subbasin id
                buf[2] = qOutlet;// flow out of subbasin
                buf[3] = t;
                MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);
            }

            //cout << rank << ":" << pTasks[index] << "End of sourceBasins loop.\n";
        }
        //if(rank == R)
        //	cout << "RANK:" << rank << " step 1.2\n";

        if (!includeChannel)
            continue;

        // 1.2 the slope routing of downstream subbasins
        for (set<int>::iterator it = downStreamSet.begin(); it != downStreamSet.end(); ++it)
        {
            int index = *it;
            //cout << "RANK:" << rank << " " << index << endl;
            ModelMain *pSubbasin = modelList[index];
            //if(!pSubbasin->IsInitialized())
            //{
            //    SettingsInput *input = new SettingsInput(inputFile, conn, db, pTasks[index]);
            //    pSubbasin->Init(input, nThreads);
            //}
            for (int i = 0; i < nHs; ++i)
                pSubbasin->StepHillSlope(curTime + i * dtHs, yearIdx, i);
        }
        tTask2 = MPI_Wtime();
        tSlope = tSlope + tTask2 - tTask1;

        tTask1 = MPI_Wtime();
        //cout << "rank: " << rank << "  step 2" << endl;
        // 2. the channel routing of  downStream subbasins are calculated
        // if their upstream subbasins are already calculated
        set<int> toDoSet, canDoSet;
        //cout << "test rank: " << rank << " >> ";
        for (set<int>::iterator it = downStreamSet.begin(); it != downStreamSet.end(); it++)
        {
            toDoSet.insert(*it);
            //cout << pTasks[*it] << " ";
        }
        //cout << endl;
        while (!toDoSet.empty())
        {
            // find all subbasins that the channel routing can be done without asking the master process
            for (set<int>::iterator it = toDoSet.begin(); it != toDoSet.end();)
            {
                int index = *it;

                bool upFinished = true;
                //ostringstream oss;
                //oss << "rank: " << rank << " id: " << pTasks[index] << "  nUps:" << pUpNums[index] << " ups: ";
                for (int j = 0; j < pUpNums[index]; ++j)
                {
                    int upId = pUpStream[index * MAX_UPSTREAM + j];
                    //oss << upId << ", ";
                    // if can not find upstreams, this subbasin can not be done
                    if (qMap.find(upId) == qMap.end())
                    {
                        upFinished = false;
                        break;
                    }
                }
                //oss << endl;
                //cout << oss.str();

                if (upFinished)
                {
                    canDoSet.insert(index);
                    toDoSet.erase(it++);
                }
                else
                    it++;
            }

#ifdef DEBUG_OUTPUT
            cout << "rank" << rank << "  todo set: ";
            for(set<int>::iterator it = toDoSet.begin(); it != toDoSet.end(); ++it)
                cout << pTasks[*it] << " ";
            cout << endl;
            //cout << "rank: " << rank << "  cando set: ";
            //for(set<int>::iterator it = canDoSet.begin(); it != canDoSet.end(); ++it)
            //	cout << pTasks[*it] << " ";
            //cout << endl;
#endif
            // if can not find subbasins to calculate according to local information,
            // ask the master process if there are new upstream subbasins calculated
            if (canDoSet.empty())
            {
                buf[0] = 2.f;
                buf[1] = groupId;
                buf[2] = rank;

                MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);

                int msgLen;
                MPI_Irecv(&msgLen, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);


                float *pData = new float[msgLen];
                MPI_Irecv(pData, msgLen, MPI_FLOAT, MASTER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);


                //cout << "recv rank" << rank << "  num:" << msgLen/2 << " data:";
                for (int j = 0; j < msgLen; j += 2)
                {
                    //cout << pData[j] << " ";
                    qMap[(int) pData[j]] = pData[j + 1];
                }
                //cout << endl;
                //cout << "rank" << rank << " qMap: ";
                //for (map<int,float>::iterator it = qMap.begin(); it != qMap.end(); it++)
                //{
                //	cout << it->first << " ";
                //}
                //cout << endl;

                delete pData;
            }
            else
            {
                //cout << "rank:" << rank << endl;
                // sort according to the distance to outlet descendent
                vector<int> vec;
                set<int>::iterator it, itMax;
                while (!canDoSet.empty())
                {
                    itMax = canDoSet.begin();
                    for (it = canDoSet.begin(); it != canDoSet.end(); ++it)
                    {
                        if (pDis[*it] > pDis[*itMax])
                            itMax = it;
                    }
                    vec.push_back(*itMax);
                    canDoSet.erase(itMax);
                }

                for (vector<int>::iterator it = vec.begin(); it != vec.end(); ++it)
                {
                    int index = *it;
                    ModelMain *pSubbasin = modelList[index];

                    //cout << "index:" << index << endl;
                    float overFlowIn = 0.f;
                    for (int j = 0; j < pUpNums[index]; ++j)
                    {
                        int upId = pUpStream[index * MAX_UPSTREAM + j];
                        overFlowIn += qMap[upId];
                    }
                    pSubbasin->SetChannelFlowIn(overFlowIn);
                    pSubbasin->StepChannel(curTime, yearIdx);
                    pSubbasin->Output(curTime);

                    float qOutlet = pSubbasin->GetQOutlet();

                    //if(slaveRank == 7) cout << "rank: " << slaveRank << " id: " << pTasks[index] << "  downStream:" << pDownStream[index] << endl;
                    if (downStreamIdSet.find(pDownStream[index]) != downStreamIdSet.end())
                    {
                        qMap[pTasks[index]] = qOutlet;
                        continue;
                    }
                    // transfer the result to the master process
                    buf[0] = 1.f;  // message type
                    buf[1] = pTasks[index];   // subbasin id
                    buf[2] = qOutlet;// flow out of subbasin
                    buf[3] = t;
                    MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                }

                //cout << "rank" << rank << " qMap after execute: ";
                //for(map<int, float>::iterator it = qMap.begin(); it != qMap.end(); ++it)
                //	cout << it->first << " ";
                //cout << endl;
            }
        }
        tTask2 = MPI_Wtime();
        tChannel = tChannel + tTask2 - tTask1;

        MPI_Barrier(slaveComm);
        if (slaveRank == 0)
        {
            buf[0] = 0.f;
            MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
        }
        MPI_Barrier(slaveComm);
    }


    //delete qs;
    //qFile.close();
    t2 = MPI_Wtime();
    t = t2 - t1;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0)
    {
        double computingTime = Max(tReceive, nSlaves);
        //cout << "[DEBUG]\tnprocs: " << numprocs-1 << " Max: " << computingTime << "   Total: " << Sum(tReceive, nSlaves) << "\n";
        cout << "[DEBUG][TIMESPAN][COMPUTING]" << computingTime << "\n";
        //cout << "[DEBUG][TIMESPAN][TOTAL]" << computingTime + ioTime << "\n";
    }

    t1 = MPI_Wtime();
    for (int i = 0; i < nSubbasins; i++)
    {
        modelList[i]->Output();
        modelList[i]->CloseGridFS();
    }
    t2 = MPI_Wtime();
    t = t2 - t1;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0)
    {
        double outputTime = Max(tReceive, nSlaves);
        //cout << "[DEBUG][TIMESPAN][OUTPUT]" << outputTime << "\n";
    }

    double tEnd = MPI_Wtime();
    t = tEnd - tStart;
    MPI_Gather(&t, 1, MPI_DOUBLE, tReceive, 1, MPI_DOUBLE, 0, slaveComm);
    if (slaveRank == 0)
    {
        double allTime = Max(tReceive, nSlaves);
        cout << "[DEBUG][TIMESPAN][TOTAL]" << allTime << "\n";
    }

    //cout << "Rank: " << slaveRank << "\ttime:\t" << tSlope << "\t" << tChannel << endl;
    MPI_Barrier(slaveComm);
    // tell the master process to exit
    if (slaveRank == 0)
    {
        buf[0] = 9.f;
        MPI_Isend(buf, MSG_LEN, MPI_FLOAT, MASTER_RANK, WORK_TAG, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);
    }

    // clean up
    for (int i = 0; i < nSubbasins; i++)
    {
        if (modelList[i] != NULL)
            delete modelList[i];
    }

    delete[] pTasks;
    delete[] pRanks;
    delete[] pDis;
    delete[] pDownStream;
    delete[] pUpNums;
    delete[] pUpStream;

    delete[] tReceive;

    //delete input;
    delete factory;
}
