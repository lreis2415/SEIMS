#include "parallel.h"

int MasterProcess(map<int, SubbasinStruct *> &subbasinMap, set<int> &groupSet) {
    StatusMessage("Enter master process...");
    MPI_Request request;
    MPI_Status status;
    int nSlaves = (int) groupSet.size();  /// groupSet normally equals (0, 1, 2, ... , nSlaves-1)
    // get the subbasin id list of different groups
    map<int, vector<int> > groupMap;
    for (auto it = groupSet.begin(); it != groupSet.end(); it++) {
        groupMap.insert(make_pair(*it, vector<int>()));
    }
    // get the subbasin ID list of different groups
    for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
        groupMap[it->second->group].push_back(it->second->id);
    }
    // get the maximum length of the task assignment message
    size_t maxTaskLen = 0;
    for (auto it = groupSet.begin(); it != groupSet.end(); it++) {
        if (groupMap[*it].size() > maxTaskLen) {
            maxTaskLen = groupMap[*it].size();
        }
    }
#ifdef _DEBUG
    cout << "Group set: " << endl;
    for (auto it = groupMap.begin(); it != groupMap.end(); it++) {
        cout << "  group id: " << it->first << ", subbasin IDs: ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            cout << *it2 << ", ";
        }
        cout << endl;
    }
    cout << "  max task length: " << maxTaskLen << endl;
#endif /* _DEBUG */

    int nTaskAll = maxTaskLen * nSlaves;
    int *pSendGroupId = nullptr; // id of the group

    int *pSendTask = nullptr; // id of subbasins
    int *pSendUpdownOrd = nullptr; // layering method of up-down stream
    int *pSendDownupOrd = nullptr; // layering method of down-up stream from outlet subbasin
    int *pSendDownStream = nullptr; // id of downstream subbasins

    int *pSendUpNums = nullptr; // number of upstream subbasins
    int *pSendUpStream = nullptr; // ids of upstream subbasins

    // initialization
    Initialize1DArray(nSlaves, pSendGroupId, -1);

    Initialize1DArray(nTaskAll, pSendTask, -1);
    Initialize1DArray(nTaskAll, pSendUpdownOrd, -1);
    Initialize1DArray(nTaskAll, pSendDownupOrd, -1);
    Initialize1DArray(nTaskAll, pSendDownStream, -1);

    Initialize1DArray(nTaskAll, pSendUpNums, 0);
    Initialize1DArray(nTaskAll * MAX_UPSTREAM, pSendUpStream, -1);

    int iGroup = 0;
    for (auto it = groupSet.begin(); it != groupSet.end(); it++) {
        pSendGroupId[iGroup] = *it;
        int groupIndex = iGroup * maxTaskLen;
        for (size_t i = 0; i < groupMap[*it].size(); i++) {
            int id = groupMap[*it][i];
            pSendTask[groupIndex + i] = id;
            pSendUpdownOrd[groupIndex + i] = subbasinMap[id]->updown_order;
            pSendDownupOrd[groupIndex + i] = subbasinMap[id]->downup_order;
            if (subbasinMap[id]->downStream != nullptr) {
                pSendDownStream[groupIndex + i] = subbasinMap[id]->downStream->id;
            }
            int nUps = subbasinMap[id]->upStreams.size();
            pSendUpNums[groupIndex + i] = nUps;
            if (nUps > MAX_UPSTREAM) {
                cout << "The number of upstreams exceeds MAX_UPSTREAM(4)." << endl;
                MPI_Abort(MCW, 1);
            }
            for (int j = 0; j < nUps; j++) {
                pSendUpStream[MAX_UPSTREAM * (groupIndex + i) + j] = subbasinMap[id]->upStreams[j]->id;
            }
        }
        iGroup++;
    }

    // send the information to slave0
    StatusMessage("Sending tasks to the first slave process...");
#ifdef _DEBUG
    cout << "  pSendTask, pSendUpdownOrd, pSendDownupOrd, pSendDownStream, pSendUpNums" << endl;
    for (int i = 0; i < nTaskAll; i++) {
        cout << "  " << pSendTask[i] << ", " << pSendUpdownOrd[i] << ", " << pSendDownupOrd[i] << ", "
             << pSendDownStream[i] << ", " << pSendUpNums[i] << ", " << endl;
    }
#endif /* _DEBUG */
    MPI_Send(&nTaskAll, 1, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendGroupId, nSlaves, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendTask, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendUpdownOrd, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendDownupOrd, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendDownStream, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendUpNums, nTaskAll, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(pSendUpStream, nTaskAll * MAX_UPSTREAM, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    StatusMessage("Tasks are dispatched.");

    // loop to receive information from slave process
    // first of all, receive the count of transferred values
    int transferCount;
    MPI_Irecv(&transferCount, 1, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW, &request);
    MPI_Wait(&request, &status);
#ifdef _DEBUG
    cout << "Master process received transfer values count: " << transferCount << endl;
#endif
    // initialize the transferred values for each subbasin struct
    for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
        it->second->transfer_count = transferCount;
        if (it->second->transfer_values != nullptr) { continue; }
        Initialize1DArray(transferCount, it->second->transfer_values, NODATA_VALUE);
    }

    bool finished = false;
    int buflen = MSG_LEN + transferCount;
    float *buf = nullptr;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    map<int, int> waitingMap;  // key: GroupIndex, value: Slave Rank ID

    while (!finished) {
        MPI_Irecv(buf, buflen, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &request);
        MPI_Wait(&request, &status);
        // deal with different types of message
        int msgCode = int(buf[0]);
        if (msgCode == 1) {  // transferred values of subbasins, no need to reply
#ifdef _DEBUG
            cout << "master received info: msgCode: 1, subbasin ID: " << int(buf[1]) << endl;
#endif
            int id = int(buf[1]); // subbasin id
            time_t t = int(buf[2]);
            for (int vi = 0; vi < transferCount; vi++) {
                subbasinMap[id]->transfer_values[vi] = buf[vi + MSG_LEN];
            }
            subbasinMap[id]->calculated = true;
#ifdef _DEBUG
            cout << "received data of subbasin ID: " << id << ", all: ";
            for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
                if (it->second->calculated) cout << it->first << " ";
            }
            cout << endl;
#endif
            // check waiting list
            int found = false;
            for (auto it = waitingMap.begin(); it != waitingMap.end(); it++) {
                int gid = it->first;
                int sRank = it->second;
                vector<int> &subs = groupMap[gid];
                for (size_t i = 0; i < subs.size(); i++) {
                    if (subbasinMap[id]->downStream->id != subs[i]) continue;
                    // send message to the slave process
                    int msgLen = transferCount + 1;
                    MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                    float *pData = nullptr;
                    Initialize1DArray(transferCount + 1, pData, NODATA_VALUE);
                    pData[0] = (float) id;
                    for (int vi = 0; vi < transferCount; vi++) {
                        pData[vi + 1] = subbasinMap[id]->transfer_values[vi];
                    }
                    MPI_Isend(pData, transferCount + 1, MPI_FLOAT, sRank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
#ifdef _DEBUG
                    cout << "active >> " << pData[0] << "->" << sRank << endl;
#endif
                    found = true;
                    // delete the current group from waiting group
                    waitingMap.erase(it);
                    subbasinMap[id]->calculated = false;  // for next timestep
                    Release1DArray(pData);
                    break;
                }
                if (found) {
                    break;
                }
            }
        } else if (msgCode == 2) {
            // a slave process is asking for information of the newly calculated upstream subbasins
#ifdef _DEBUG
            cout << "master received info: msgCode: 2, group: " << int(buf[1]) << ", from rank: " << int(buf[2])
                 << endl;
#endif
            map<int, float *> transMap; // used to contain flowout of the newly calculated basins
            int gid = int(buf[1]);
            int sRank = int(buf[2]);
            vector<int> &subs = groupMap[gid];
            // loop subbasins in the group
            for (size_t i = 0; i < subs.size(); i++) {
                int id = subs[i];
                // for not most upstream basins
                if (subbasinMap[id]->updown_order > 1) {
                    // find if their upstream basins are newly calculated
                    vector<SubbasinStruct *> &ups = subbasinMap[id]->upStreams;
                    for (size_t j = 0; j < ups.size(); j++) {
                        if (ups[j]->calculated) {
                            //transMap[ups[j]->id] = ups[j]->qOutlet;
                            transMap[ups[j]->id] = ups[j]->transfer_values;
                            ups[j]->calculated = false; // for next timestep
                        }
                    }
                }
            }
            if (transMap.empty()) {
                waitingMap[gid] = sRank;
            } else {
                // tell the slave process the message length containing new information
                int msgLen = transMap.size() * (transferCount + 1);
                MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MCW, &request);
                float *pData = nullptr;
                Initialize1DArray(msgLen, pData, NODATA_VALUE);
                int counter = 0;
                for (auto it = transMap.begin(); it != transMap.end(); it++) {
                    pData[(transferCount + 1) * counter] = (float) it->first;
                    for (int vi = 0; vi < transferCount; vi++) {
                        pData[(transferCount + 1) * counter + vi + 1] = it->second[vi];
                    }
                    counter++;
                }
                MPI_Wait(&request, &status);
                MPI_Isend(pData, msgLen, MPI_FLOAT, sRank, WORK_TAG, MCW, &request);
                MPI_Wait(&request, &status);

#ifdef _DEBUG
                cout << "positive >> ";
                for (int i = 0; i < msgLen; i += (transferCount + 1)) {
                    cout << "subbasinID: " << pData[i] << " -> world_rand: " << sRank << " ";
                }
                cout << endl;
#endif
                delete[] pData;
            }
        } else if (msgCode == 0) {  // reset for new timestep
            for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
                it->second->calculated = false;
                for (int vi = 0; vi < it->second->transfer_count; vi++) {
                    it->second->transfer_values[vi] = NODATA_VALUE;
                }
            }
            StatusMessage("master: running to next timestep...");
        } else if (msgCode == 9) {
            finished = true;
            StatusMessage("Exit from the master process.");
        }
    }
    Release1DArray(buf);
    Release1DArray(pSendGroupId);
    Release1DArray(pSendTask);
    Release1DArray(pSendUpdownOrd);
    Release1DArray(pSendDownupOrd);
    Release1DArray(pSendDownStream);
    Release1DArray(pSendUpNums);
    Release1DArray(pSendUpStream);
    return 0;
}
