#include "TaskInformation.h"
#include "utils_array.h"
#include "parallel.h"

using namespace ccgl::utils_array;
using std::make_pair;

TaskInfo::TaskInfo(const int size, const int rank):
    max_len(-1), subbsn_count(-1), subbsn_id(nullptr),
    lyr_id(nullptr), down_id(nullptr), up_count(nullptr),
    up_ids(nullptr), size_(size),
    rank_(rank), subbsn_count_rank_(nullptr), max_lyr_(-1), max_lyr_all_(-1) {

}

TaskInfo::~TaskInfo() {
    if (subbsn_id != nullptr) Release1DArray(subbsn_id);
    if (lyr_id != nullptr) Release1DArray(lyr_id);
    if (down_id != nullptr) Release1DArray(down_id);
    if (up_count != nullptr) Release1DArray(up_count);
    if (up_ids != nullptr) Release1DArray(up_ids);
    if (subbsn_count_rank_ != nullptr) Release1DArray(subbsn_count_rank_);
    for (auto it = subbsn_tfvalues_.begin(); it != subbsn_tfvalues_.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (it2->second != nullptr) {
                Release1DArray(it2->second);
                it2->second = nullptr;
            }
            it->second.erase(it2++);
        }
        subbsn_tfvalues_.erase(it++);
    }
    for (auto it = recv_subbsn_tfvalues_.begin(); it != recv_subbsn_tfvalues_.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (it2->second != nullptr) {
                Release1DArray(it2->second);
                it2->second = nullptr;
            }
            it->second.erase(it2++);
        }
        recv_subbsn_tfvalues_.erase(it++);
    }
}

bool TaskInfo::CheckInputData() {
    return max_len > 0 && subbsn_id != nullptr && lyr_id != nullptr &&
            down_id != nullptr && up_count != nullptr && up_ids != nullptr;
}

bool TaskInfo::Build() {
    if (!CheckInputData()) return false;
    /// If Build() has already been invoked.
    if (nullptr != subbsn_count_rank_ && !rank_subbsn_id_.empty() &&
        !subbsn_rank_.empty() && !subbsn_layer_.empty() && !downstream_.empty() &&
        !upstreams_.empty() && !upstreams_inrank_.empty() && !lyr_subbsns_.empty() &&
        !srclyr_subbsns_.empty() && !nonsrclyr_subbsns_.empty())
        return true;
    /// rank_subbsn_id_, subbsn_count_rank_, subbsn_rank_, and downstream_
    Initialize1DArray(size_, subbsn_count_rank_, 0);
    for (int irank = 0; irank < size_; irank++) {
        for (int i = 0; i < max_len; i++) {
            if (subbsn_id[irank * max_len + i] < 0) continue;
            subbsn_count_rank_[irank]++;
            subbsn_rank_[subbsn_id[irank * max_len + i]] = irank;
        }
    }
    for (int i = rank_ * max_len; i < (rank_ + 1) * max_len; i++) {
        if (subbsn_id[i] < 0) continue;
        rank_subbsn_id_.emplace_back(subbsn_id[i]);
        downstream_[subbsn_id[i]] = down_id[i] > 0 ? down_id[i] : -1;
    }
#ifdef _DEBUG
    cout << "Subbasin ID -> Rank ID" << endl;
    for (auto it = subbsn_rank_.begin(); it != subbsn_rank_.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
#endif
    /// subbsn_layer_, lyr_subbsns_, srclyr_subbsns_ and nonsrclyr_subbsns_
    max_lyr_all_ = 0;
    for (int i = 0; i < size_ * max_len; i++) {
        if (subbsn_id[i] < 0) continue;
        if (lyr_id[i] > max_lyr_all_) max_lyr_all_ = lyr_id[i];
        subbsn_layer_[subbsn_id[i]] = lyr_id[i];
    }
    max_lyr_ = 0;
    for (int i = 0; i < subbsn_count_rank_[rank_]; i++) {
        int sub_idx = rank_ * max_len + i;
        int sub_id = subbsn_id[sub_idx];
        // classification according to the Layering method, i.e., up-down and down-up orders.
        int stream_order = lyr_id[sub_idx];
        if (stream_order > max_lyr_) { max_lyr_ = stream_order; }
        if (lyr_subbsns_.find(stream_order) == lyr_subbsns_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
            lyr_subbsns_.emplace(stream_order, vector<int>());
#else
            lyr_subbsns_.insert(make_pair(stream_order, vector<int>()));
#endif
        }
        lyr_subbsns_[stream_order].emplace_back(sub_id);
        if (up_count[sub_idx] == 0) {
            // source subbasins of each layer
            if (srclyr_subbsns_.find(stream_order) == srclyr_subbsns_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
                srclyr_subbsns_.emplace(stream_order, vector<int>());
#else
                srclyr_subbsns_.insert(make_pair(stream_order, vector<int>()));
#endif
            }
            srclyr_subbsns_[stream_order].emplace_back(sub_id);
        } else {
            if (nonsrclyr_subbsns_.find(stream_order) == nonsrclyr_subbsns_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
                nonsrclyr_subbsns_.emplace(stream_order, vector<int>());
#else
                nonsrclyr_subbsns_.insert(make_pair(stream_order, vector<int>()));
#endif
            }
            nonsrclyr_subbsns_[stream_order].emplace_back(sub_id);
        }
    }
#ifdef _DEBUG
    cout << "Rank: " << rank_ << ", Source subbasins: " << endl;
    for (auto it = srclyr_subbsns_.begin(); it != srclyr_subbsns_.end(); ++it) {
        cout << "    Layer ID: " << it->first << ": ";
        for (auto it_id = it->second.begin(); it_id != it->second.end(); ++it_id) {
            cout << *it_id << ", ";
        }
        cout << endl;
    }
    cout << endl;
    cout << "Rank: " << rank_ << ", Non-Source subbasins: " << endl;
    for (auto it = nonsrclyr_subbsns_.begin(); it != nonsrclyr_subbsns_.end(); ++it) {
        cout << "    Layer ID: " << it->first << ": ";
        for (auto it_id = it->second.begin(); it_id != it->second.end(); ++it_id) {
            cout << *it_id << ", ";
        }
        cout << endl;
    }
    cout << endl;
#endif /* _DEBUG */

    /// upstreams_ and upstreams_inrank_
    for (int irank = 0; irank < size_; irank++) {
        for (int i = 0; i < max_len; i++) {
            int subid = subbsn_id[irank * max_len + i];
            if (subid < 0) continue;
            upstreams_inrank_[subid] = true; // No matter a subbasin has upstreams or not.
            if (upstreams_.find(subid) == upstreams_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
                upstreams_.emplace(subid, vector<int>());
#else
                upstreams_.insert(make_pair(subid, vector<int>()));
#endif
            }
            for (int j = 0; j < up_count[irank * max_len + i]; j++) {
                int up_id = up_ids[irank * max_len * MAX_UPSTREAM + i * MAX_UPSTREAM + j];
                upstreams_[subid].emplace_back(up_id);
                if (subbsn_rank_[up_id] != irank) {
                    upstreams_inrank_[subid] = false;
                }
            }
        }
    }
#ifdef _DEBUG
    cout << "Subbasin ID -> Upstreams" << endl;
    for (auto it = upstreams_.begin(); it != upstreams_.end(); ++it) {
        cout << it->first << " -> ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            cout << *it2 << ", ";
        }
        cout << endl;
    }
    cout << "Subbasin ID -> Is all upstream subbasins in the same rank?" << endl;
    for (auto it = upstreams_inrank_.begin(); it != upstreams_inrank_.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
#endif /* _DEBUG */
    return true;
}

void TaskInfo::MallocTransferredValues(const int transfer_count, const int multiplier) {
    for (int i = 1; i <= max_lyr_all_ * multiplier; i++) {
        // i is simulation sequence
        for (int j = 1; j <= max_lyr_all_; j++) {
            if (lyr_subbsns_.find(j) == lyr_subbsns_.end()) continue;
            for (auto it = lyr_subbsns_[j].begin(); it != lyr_subbsns_[j].end(); ++it) {
                if (downstream_[*it] < 0) continue; // No need to malloc space for outlet subbasin
                if (subbsn_tfvalues_.find(i) == subbsn_tfvalues_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
                    subbsn_tfvalues_.emplace(i, map<int, float *>());
#else
                    subbsn_tfvalues_.insert(make_pair(i, map<int, float *>()));
#endif
                }
                float* tfvalues = nullptr;
                Initialize1DArray(transfer_count, tfvalues, NODATA_VALUE);
#ifdef HAS_VARIADIC_TEMPLATES
                subbsn_tfvalues_[i].emplace(*it, tfvalues);
#else
                subbsn_tfvalues_[i].insert(make_pair(*it, tfvalues));
#endif
            }
        }
    }

    for (auto it_id = upstreams_.begin(); it_id != upstreams_.end(); ++it_id) {
        if (subbsn_rank_[it_id->first] != rank_) continue;
        for (auto it_up = it_id->second.begin(); it_up != it_id->second.end(); ++it_up) {
            if (subbsn_rank_[*it_up] == rank_) continue;
            for (int i = 1; i <= max_lyr_all_ * multiplier; i++) {
                if (recv_subbsn_tfvalues_.find(i) == recv_subbsn_tfvalues_.end()) {
#ifdef HAS_VARIADIC_TEMPLATES
                    recv_subbsn_tfvalues_.emplace(i, map<int, float *>());
#else
                    recv_subbsn_tfvalues_.insert(make_pair(i, map<int, float *>()));
#endif
                }
                float* tfvalues = nullptr;
                Initialize1DArray(transfer_count, tfvalues, NODATA_VALUE);
#ifdef HAS_VARIADIC_TEMPLATES
                recv_subbsn_tfvalues_[i].emplace(*it_up, tfvalues);
#else
                recv_subbsn_tfvalues_[i].insert(make_pair(*it_up, tfvalues));
#endif
            }
        }
    }
}

int TaskInfo::GetSubbasinNumber() {
    if (!CheckInputData()) return -1;
    if (nullptr == subbsn_count_rank_) Build();
    return subbsn_count_rank_[rank_];
}
