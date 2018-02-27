#include "parallel.h"
#include "utilities.h"

//float deepGw = 0.f;

int MasterProcess(map<int, SubbasinStruct *> &subbasinMap, set<int> &groupSet, InputArgs *input_args) {
    StatusMessage("Enter master process...");
    MPI_Request request;
    MPI_Status status;
    int nSlaves = groupSet.size();  /// groupSet normally equals (0, 1, 2, ... , nSlaves-1)
    // get the subbasin id list of different groups
    map<int, vector<int> > groupMap;
    for (auto it = groupSet.begin(); it != groupSet.end(); it++) {
        groupMap.insert(make_pair(*it, vector<int>()));
    }
    int idOutlet = -1;
    for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
        groupMap[it->second->group].push_back(it->second->id);
        if (nullptr == it->second->downStream) {
            idOutlet = it->second->id;
        }
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
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
            cout << *it2 << ", ";
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
    bool finished = false;
    float buf[MSG_LEN];
    map<int, int> waitingMap;  // key: GroupIndex, value: Slave Rank ID
    //ofstream fOutput(output_q_file.c_str());
    while (!finished) {
        MPI_Irecv(&buf, MSG_LEN, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &request);
        MPI_Wait(&request, &status);
        StatusMessage(("master received info: msgCode: " + ValueToString(int(buf[0])) + 
                       ", subbasin ID: " + ValueToString(int(buf[1]))).c_str());
        // deal with different types of message
        int msgCode = int(buf[0]);
        if (msgCode == 1) {  // outlet flowout of subbasins, no need to reply
            int id = int(buf[1]); // subbasin id
            //cout << "master: " << id << endl;
            subbasinMap[id]->qOutlet = buf[2];
            subbasinMap[id]->calculated = true;
            time_t t = int(buf[3]);
#ifdef _DEBUG
            cout << "subbasins >> in: " << id << "  all: ";
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
                    int msgLen = 2;
                    MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MCW, &request);
                    float pData[2];
                    pData[0] = (float) id;
                    pData[1] = subbasinMap[id]->qOutlet;
                    MPI_Wait(&request, &status);
                    MPI_Isend(pData, msgLen, MPI_FLOAT, sRank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
#ifdef _DEBUG
                    cout << "active >> " << pData[0] << "->" << sRank << endl;
#endif
                    found = true;

                    // delete the current group from waiting group
                    waitingMap.erase(it);
                    subbasinMap[id]->calculated = false;  // for next timestep

                    break;
                }
                if (found) {
                    break;
                }
            }
        } else if (msgCode == 2) {
            // a slave process is asking for information of the newly calculated upstream subbasins
            map<int, float> transMap; // used to contain flowout of the newly calculated basins
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
                            transMap[ups[j]->id] = ups[j]->qOutlet;
                            ups[j]->calculated = false;
                        }
                    }
                }
            }

            if (transMap.empty()) {
                waitingMap[gid] = sRank;
            } else {
                // tell the slave process the message length containing new information
                int msgLen = transMap.size() * 2;
                MPI_Isend(&msgLen, 1, MPI_INT, sRank, WORK_TAG, MCW, &request);
                float *pData = new float[msgLen];
                int counter = 0;
                for (auto it = transMap.begin(); it != transMap.end(); it++) {
                    pData[2 * counter] = (float) it->first;
                    pData[2 * counter + 1] = it->second;
                    counter++;
                }
                MPI_Wait(&request, &status);
                MPI_Isend(pData, msgLen, MPI_FLOAT, sRank, WORK_TAG, MCW, &request);
                MPI_Wait(&request, &status);

#ifdef _DEBUG
                cout << "positive >> ";
                for(int i = 0; i < msgLen; i += 2)
                    cout << pData[i] << "->" << sRank << " ";
                cout << endl;
#endif
                delete[] pData;
            }
        } else if (msgCode == 0) {  // reset for new timestep
            for (auto it = subbasinMap.begin(); it != subbasinMap.end(); it++) {
                it->second->calculated = false;
                it->second->qOutlet = 0.f;
            }
            StatusMessage("master: running to next timestep...");
        } else if (msgCode == 9) {
            finished = true;
            StatusMessage("Exit from the master process.");
        }
    }

    Release1DArray(pSendGroupId);

    Release1DArray(pSendTask);
    Release1DArray(pSendUpdownOrd);
    Release1DArray(pSendDownupOrd);
    Release1DArray(pSendDownStream);

    Release1DArray(pSendUpNums);
    Release1DArray(pSendUpStream);

    return 0;
}
