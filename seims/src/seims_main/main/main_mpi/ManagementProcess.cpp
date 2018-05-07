#include "parallel.h"

int MasterProcess(map<int, SubbasinStruct *>& subbasin_map, set<int>& group_set) {
    StatusMessage("Enter master process...");
    MPI_Request request;
    MPI_Status status;
    int nslaves = CVT_INT(group_set.size()); /// groupSet normally equals (0, 1, 2, ... , nSlaves-1)
    // get the subbasin id list of different groups
    map<int, vector<int> > group_map;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        group_map.insert(make_pair(*it, vector<int>()));
    }
    // get the subbasin ID list of different groups
    for (auto it = subbasin_map.begin(); it != subbasin_map.end(); ++it) {
        group_map[it->second->group].push_back(it->second->id);
    }
    // get the maximum length of the task assignment message
    int max_task_len = 0;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        if (group_map[*it].size() > max_task_len) {
            max_task_len = CVT_INT(group_map[*it].size());
        }
    }
#ifdef _DEBUG
    cout << "Group set: " << endl;
    for (auto it = group_map.begin(); it != group_map.end(); ++it) {
        cout << "  group id: " << it->first << ", subbasin IDs: ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            cout << *it2 << ", ";
        }
        cout << endl;
    }
    cout << "  max task length: " << max_task_len << endl;
#endif /* _DEBUG */

    int n_task_all = max_task_len * nslaves;
    int* p_send_group_id = nullptr; // id of the group

    int* p_send_task = nullptr;        // id of subbasins
    int* p_send_updown_ord = nullptr;  // layering method of up-down stream
    int* p_send_downup_ord = nullptr;  // layering method of down-up stream from outlet subbasin
    int* p_send_down_stream = nullptr; // id of downstream subbasins

    int* p_send_up_nums = nullptr;   // number of upstream subbasins
    int* p_send_up_stream = nullptr; // ids of upstream subbasins

    // initialization
    Initialize1DArray(nslaves, p_send_group_id, -1);

    Initialize1DArray(n_task_all, p_send_task, -1);
    Initialize1DArray(n_task_all, p_send_updown_ord, -1);
    Initialize1DArray(n_task_all, p_send_downup_ord, -1);
    Initialize1DArray(n_task_all, p_send_down_stream, -1);

    Initialize1DArray(n_task_all, p_send_up_nums, 0);
    Initialize1DArray(n_task_all * MAX_UPSTREAM, p_send_up_stream, -1);

    int igroup = 0;
    for (auto it = group_set.begin(); it != group_set.end(); ++it) {
        p_send_group_id[igroup] = *it;
        int group_index = igroup * max_task_len;
        for (size_t i = 0; i < group_map[*it].size(); i++) {
            int id = group_map[*it][i];
            p_send_task[group_index + i] = id;
            p_send_updown_ord[group_index + i] = subbasin_map[id]->updown_order;
            p_send_downup_ord[group_index + i] = subbasin_map[id]->downup_order;
            if (subbasin_map[id]->down_stream != nullptr) {
                p_send_down_stream[group_index + i] = subbasin_map[id]->down_stream->id;
            }
            int n_ups = CVT_INT(subbasin_map[id]->up_streams.size());
            p_send_up_nums[group_index + i] = n_ups;
            if (n_ups > MAX_UPSTREAM) {
                cout << "The number of upstreams exceeds MAX_UPSTREAM(4)." << endl;
                MPI_Abort(MCW, 1);
            }
            for (int j = 0; j < n_ups; j++) {
                p_send_up_stream[MAX_UPSTREAM * (group_index + i) + j] = subbasin_map[id]->up_streams[j]->id;
            }
        }
        igroup++;
    }

    // send the information to slave0
    StatusMessage("Sending tasks to the first slave process...");
#ifdef _DEBUG
    cout << "  pSendTask, pSendUpdownOrd, pSendDownupOrd, pSendDownStream, pSendUpNums" << endl;
    for (int i = 0; i < n_task_all; i++) {
        cout << "  " << p_send_task[i] << ", " << p_send_updown_ord[i] << ", " << p_send_downup_ord[i]
                << ", " << p_send_down_stream[i] << ", " << p_send_up_nums[i] << ", " << endl;
    }
#endif /* _DEBUG */
    MPI_Send(&n_task_all, 1, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_group_id, nslaves, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_task, n_task_all, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_updown_ord, n_task_all, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_downup_ord, n_task_all, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_down_stream, n_task_all, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_up_nums, n_task_all, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    MPI_Send(p_send_up_stream, n_task_all * MAX_UPSTREAM, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW);
    StatusMessage("Tasks are dispatched.");

    // loop to receive information from slave process
    // first of all, receive the count of transferred values
    int transfer_count;
    MPI_Irecv(&transfer_count, 1, MPI_INT, SLAVE0_RANK, WORK_TAG, MCW, &request);
    MPI_Wait(&request, &status);
#ifdef _DEBUG
    cout << "Master process received transfer values count: " << transfer_count << endl;
#endif
    // initialize the transferred values for each subbasin struct
    for (auto it = subbasin_map.begin(); it != subbasin_map.end(); ++it) {
        it->second->transfer_count = transfer_count;
        if (it->second->transfer_values != nullptr) { continue; }
        Initialize1DArray(transfer_count, it->second->transfer_values, NODATA_VALUE);
    }

    bool finished = false;
    int buflen = MSG_LEN + transfer_count;
    float* buf = nullptr;
    Initialize1DArray(buflen, buf, NODATA_VALUE);
    map<int, int> waiting_map; // key: GroupIndex, value: Slave Rank ID

    while (!finished) {
        MPI_Irecv(buf, buflen, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &request);
        MPI_Wait(&request, &status);
        // deal with different types of message
        int msg_code = int(buf[0]);
        if (msg_code == 1) {
            // receive transferred values of subbasin, no need to reply
#ifdef _DEBUG
            cout << "master received info: msgCode: 1, subbasin ID: " << int(buf[1]) << endl;
#endif
            int id = int(buf[1]); // subbasin id
            time_t t = int(buf[2]);
            for (int vi = 0; vi < transfer_count; vi++) {
                subbasin_map[id]->transfer_values[vi] = buf[vi + MSG_LEN];
            }
            subbasin_map[id]->calculated = true;
#ifdef _DEBUG
            cout << "received data of subbasin ID: " << id << ", all: ";
            for (auto it = subbasin_map.begin(); it != subbasin_map.end(); ++it) {
                if (it->second->calculated) cout << it->first << " ";
            }
            cout << endl;
#endif
            // check waiting list
            int found = false;
            for (auto it = waiting_map.begin(); it != waiting_map.end(); ++it) {
                int gid = it->first;
                int srank = it->second;
                for (auto isub = group_map[gid].begin(); isub != group_map[gid].end(); ++isub) {
                    if (subbasin_map[id]->down_stream->id != *isub) continue;
                    // send message to the slave process
                    int msg_len = transfer_count + 1;
                    MPI_Isend(&msg_len, 1, MPI_INT, srank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
                    float* pdata = nullptr;
                    Initialize1DArray(transfer_count + 1, pdata, NODATA_VALUE);
                    pdata[0] = float(id);
                    for (int vi = 0; vi < transfer_count; vi++) {
                        pdata[vi + 1] = subbasin_map[id]->transfer_values[vi];
                    }
                    MPI_Isend(pdata, transfer_count + 1, MPI_FLOAT, srank, WORK_TAG, MCW, &request);
                    MPI_Wait(&request, &status);
#ifdef _DEBUG
                    cout << "Send data of subbasin >> " << pdata[0] << " -> world_rank: " << srank << endl;
#endif
                    found = true;
                    // delete the current group from waiting group
                    waiting_map.erase(it);
                    subbasin_map[id]->calculated = false; // for next timestep
                    Release1DArray(pdata);
                    break;
                }
                if (found) { break; }
            }
        } else if (msg_code == 2) {
            // a slave process is asking for information of the newly calculated upstream subbasins
#ifdef _DEBUG
            cout << "master received info: msgCode: 2, group: " << int(buf[1]) <<
                    ", from world_rank: " << int(buf[2]) << endl;
#endif
            map<int, float *> trans_map; // used to contain flowout of the newly calculated basins
            int gid = int(buf[1]);
            int srank = int(buf[2]);
            // loop subbasins in the group
            for (auto isub = group_map[gid].begin(); isub != group_map[gid].end(); ++isub) {
                vector<SubbasinStruct *>& ups = subbasin_map[*isub]->up_streams;
                for (auto iup = ups.begin(); iup != ups.end(); ++iup) {
                    if ((*iup)->calculated) {
                        trans_map[(*iup)->id] = (*iup)->transfer_values;
                        (*iup)->calculated = false; // for next timestep
                    }
                }
            }
            if (trans_map.empty()) {
                waiting_map[gid] = srank;
            } else {
                // tell the slave process the message length containing new information
                int msg_len = CVT_INT(trans_map.size()) * (transfer_count + 1);
                MPI_Isend(&msg_len, 1, MPI_INT, srank, WORK_TAG, MCW, &request);
                float* pdata = nullptr;
                Initialize1DArray(msg_len, pdata, NODATA_VALUE);
                int counter = 0;
                for (auto it = trans_map.begin(); it != trans_map.end(); ++it) {
                    pdata[(transfer_count + 1) * counter] = float(it->first);
                    for (int vi = 0; vi < transfer_count; vi++) {
                        pdata[(transfer_count + 1) * counter + vi + 1] = it->second[vi];
                    }
                    counter++;
                }
                MPI_Wait(&request, &status);
                MPI_Isend(pdata, msg_len, MPI_FLOAT, srank, WORK_TAG, MCW, &request);
                MPI_Wait(&request, &status);

#ifdef _DEBUG
                cout << "Send data of subbasin >> ";
                for (int i = 0; i < msg_len; i += (transfer_count + 1)) {
                    cout << "    subbasinID: " << pdata[i] << " -> world_rand: " << srank << " ";
                }
                cout << endl;
#endif
                Release1DArray(pdata);
            }
        } else if (msg_code == 0) {
            // reset for new timestep
            for (auto it = subbasin_map.begin(); it != subbasin_map.end(); ++it) {
                it->second->calculated = false;
                for (int vi = 0; vi < it->second->transfer_count; vi++) {
                    it->second->transfer_values[vi] = NODATA_VALUE;
                }
            }
            StatusMessage("master: running to next timestep...");
        } else if (msg_code == 9) {
            finished = true;
            StatusMessage("Exit from the master process.");
        }
    }
    Release1DArray(buf);
    Release1DArray(p_send_group_id);
    Release1DArray(p_send_task);
    Release1DArray(p_send_updown_ord);
    Release1DArray(p_send_downup_ord);
    Release1DArray(p_send_down_stream);
    Release1DArray(p_send_up_nums);
    Release1DArray(p_send_up_stream);
    return 0;
}
