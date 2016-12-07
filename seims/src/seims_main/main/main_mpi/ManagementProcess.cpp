#include "parallel.h"
#include "mpi.h"
#include <iostream>
#include <iomanip>
#include <fstream>

#ifndef linux

#include "Windows.h"

#endif

#include "utils.h"

float deepGw = 0.f;

int MasterProcess(map<int, SubbasinStruct *> &subbasinMap, set<int> &groupSet, string &projectPath, string &outputFile)
{
    //cout << "Enter master process.\n";
    MPI_Request request;
    MPI_Status status;
    int nSlaves = groupSet.size();
    //cout << "nSlaves " << nSlaves << endl;
    map<int, vector<int> > groupMap;
    for (set<int>::iterator it = groupSet.begin(); it != groupSet.end(); ++it)
        groupMap[*it] = vector<int>();

    // get the subbasin id list of different groups
    int idOutlet = -1;
    for (map<int, SubbasinStruct *>::iterator it = subbasinMap.begin(); it != subbasinMap.end(); ++it)
    {
        groupMap[it->second->group].push_back(it->second->id);
        if (it->second->downStream == NULL)
            idOutlet = it->second->id;
    }
    // get the maximum length of the task assignment message
    size_t maxTaskLen = 0;
    for (set<int>::iterator it = groupSet.begin(); it != groupSet.end(); ++it)
    {
        if (groupMap[*it].size() > maxTaskLen)
            maxTaskLen = groupMap[*it].size();
    }

    int nTaskAll = maxTaskLen * groupMap.size();
    int *pSendTask = new int[nTaskAll]; // id of subbasins
    int *pSendRank = new int[nTaskAll]; // distance to the most upstream subbasin
    int *pSendDis = new int[nTaskAll];  // distance to the outlet subbasin
    int *pSendDownStream = new int[nTaskAll]; // id of downstream subbasins
    int *pSendUpNums = new int[nTaskAll];     // number of upstream subbasins
    int *pSendUpStream = new int[nTaskAll * MAX_UPSTREAM]; // ids of upstream subbasins

    int *pSendGroupId = new int[groupMap.size()]; // id of the group

    int iGroup = 0;
    for (set<int>::iterator it = groupSet.begin(); it != groupSet.end(); ++it)
    {
        pSendGroupId[iGroup] = *it;
        vector<int> &vec = groupMap[*it];
        int groupIndex = iGroup * maxTaskLen;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            int id = vec[i];
            pSendTask[groupIndex + i] = id;
            pSendRank[groupIndex + i] = subbasinMap[id]->rank;
            pSendDis[groupIndex + i] = subbasinMap[id]->disToOutlet;
            if (subbasinMap[id]->downStream != NULL)
                pSendDownStream[groupIndex + i] = subbasinMap[id]->downStream->id;
            else
                pSendDownStream[groupIndex + i] = -1;

            int nUps = subbasinMap[id]->upStreams.size();
            pSendUpNums[groupIndex + i] = nUps;
            if (nUps > MAX_UPSTREAM)
            {
                cout << "The number of upstreams exceeds MAX_UPSTREAM.\n";
                exit(-1);
            }
            for (int j = 0; j < nUps; ++j)
            {
                pSendUpStream[MAX_UPSTREAM * (groupIndex + i) + j] = subbasinMap[id]->upStreams[j]->id;
            }
            for (int j = nUps; j < MAX_UPSTREAM; ++j)
            {
                pSendUpStream[MAX_UPSTREAM * (groupIndex + i) + j] = -1;
            }
        }
        for (size_t i = vec.size(); i < maxTaskLen; ++i)
        {
            pSendTask[groupIndex + i] = -1;
            pSendRank[groupIndex + i] = -1;
            pSendDis[groupIndex + i] = -1;
            pSendDownStream[groupIndex + i] = -1;
        }
        iGroup++;
    }

    // send the information to slave0
    //cout << "Sending tasks...\n";
    //cout << "MASTER " << nTaskAll << endl;
    MPI_Send(&nTaskAll, 1, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendGroupId, nSlaves, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendTask, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendRank, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendDis, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendDownStream, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendUpNums, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);
    MPI_Send(pSendUpStream, nTaskAll * MAX_UPSTREAM, MPI_INT, SLAVE0_RANK, WORK_TAG, MPI_COMM_WORLD);

    //cout << "Tasks are dispatched.\n";

    // loop to receive information from slave process
    bool finished = false;
    float buf[MSG_LEN];
    map<int, int> waitingMap;
    ofstream fOutput(outputFile.c_str());
    while (!finished)
    {
        MPI_Irecv(&buf, MSG_LEN, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);
        //cout << "master info:" << int(buf[1]) << " " << buf[2] << endl;
        // deal with different types of message
        int msgCode = buf[0];
        if (msgCode == 1)// outlet flowout of subbasins, no need to reply
        {
            int id = int(buf[1]); // subbasin id
            //cout << "master: " << id << endl;
            subbasinMap[id]->qOutlet = buf[2];
            subbasinMap[id]->calculated = true;
            time_t t = int(buf[3]);

#ifdef DEBUG_OUTPUT
            cout << "subbasins>> in: " << id << "  all: ";
            for (map<int, SubbasinStruct*>::iterator it = subbasinMap.begin(); it != subbasinMap.end(); it++)
            {
                if (it->second->calculated)
                    cout << it->first << " ";
            }
            cout << endl;
#endif
            // check waiting list
            int found = false;
            map<int, int>::iterator it;
            for (it = waitingMap.begin(); it != waitingMap.end(); ++it)
            {
                int gid = it->first;
                int sRank = it->second;
                vector<int> &subs = groupMap[gid];
                for (size_t i = 0; i < subs.size(); i++)
                {
                    if (subbasinMap[id]->downStream->id == subs[i])
                    {
                        // send message to the slave process
                        int msgLen = 2;
                        MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MPI_COMM_WORLD, &request);
                        float pData[2];
                        pData[0] = (float) id;
                        pData[1] = subbasinMap[id]->qOutlet;
                        MPI_Wait(&request, &status);
                        MPI_Isend(pData, msgLen, MPI_FLOAT, sRank, WORK_TAG, MPI_COMM_WORLD, &request);
                        MPI_Wait(&request, &status);
#ifdef DEBUG_OUTPUT
                        cout << "active >> " << pData[0] << "->" << sRank << endl;
#endif
                        found = true;

                        // delete the current group from waiting group
                        waitingMap.erase(it);
                        subbasinMap[id]->calculated = false;

                        break;
                    }
                }

                if (found)
                    break;
            }


            if (id == idOutlet)
                fOutput << utils::ConvertToString2(&t) << "\t" << setprecision(8) <<
                subbasinMap[id]->qOutlet + deepGw << "\n";
        }
        else if (msgCode == 2) //a slave process is asking for information of the newly calculated upstream subbasins
        {
            map<int, float> transMap; // used to contain flowout of the newly calculated basins
            int gid = int(buf[1]);
            int sRank = int(buf[2]);
            vector<int> &subs = groupMap[gid];
            // loop subbasins in the group
            for (size_t i = 0; i < subs.size(); i++)
            {
                int id = subs[i];
                // for not most upstream basins
                if (subbasinMap[id]->rank > 1)
                {
                    // find if their upstream basins are newly calculated
                    vector<SubbasinStruct *> &ups = subbasinMap[id]->upStreams;
                    for (size_t j = 0; j < ups.size(); j++)
                    {
                        if (ups[j]->calculated)
                        {
                            transMap[ups[j]->id] = ups[j]->qOutlet;
                            ups[j]->calculated = false;
                        }
                    }
                }
            }

            if (transMap.empty())
            {
                waitingMap[gid] = sRank;
            }
            else
            {
                // tell the slave process the message length containing new information
                int msgLen = transMap.size() * 2;
                MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MPI_COMM_WORLD, &request);
                float *pData = new float[msgLen];
                int counter = 0;
                for (map<int, float>::iterator it = transMap.begin(); it != transMap.end(); it++)
                {
                    pData[2 * counter] = (float) it->first;
                    pData[2 * counter + 1] = it->second;

                    counter++;
                }
                MPI_Wait(&request, &status);
                MPI_Isend(pData, msgLen, MPI_FLOAT, sRank, WORK_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);

#ifdef DEBUG_OUTPUT
                //if(sRank == 1) cout << "master send to rank  " << sRank << " size:" << transMap.size() << " ";
                    cout << "positive >> ";
                    for(int i = 0; i < msgLen; i += 2)
                        cout << pData[i] << "->" << sRank << " ";
                    cout << endl;
#endif
                delete pData;
            }
        }
        else if (msgCode == 0) // reset all qOutlet informaion
        {
            for (map<int, SubbasinStruct *>::iterator it = subbasinMap.begin(); it != subbasinMap.end(); ++it)
            {
                it->second->calculated = false;
                it->second->qOutlet = 0.f;
            }
#ifdef DEBUG_OUTPUT
            cout << "master: newround" << endl;
#endif
        }
        else if (msgCode == 9)
        {
            finished = true;
            //cout << "Exit from the master process.\n";
        }
    }
    fOutput.close();

    for (map<int, SubbasinStruct *>::iterator it = subbasinMap.begin(); it != subbasinMap.end(); ++it)
    {
        delete it->second;
    }
    delete[] pSendTask;
    delete[] pSendRank;
    delete[] pSendDis;
    delete[] pSendDownStream;
    delete[] pSendUpNums;
    delete[] pSendUpStream;

    return 0;
}
