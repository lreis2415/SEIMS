/*!
 * \brief Class of parallel task information.
 * \author Liangjun Zhu
 * \changelog  2018-06-12  - lj -  Initial implementation.\n
 */
#ifndef SEIMS_MPI_TASK_INFO_H
#define SEIMS_MPI_TASK_INFO_H

#include "basic.h"

#include <map>
#include <vector>

using namespace ccgl;
using std::map;
using std::vector;

class TaskInfo: Interface {
public:
    /*! Constructor
     * \param[in] size Number of process
     * \param[in] rank Rank ID
     */
    explicit TaskInfo(int size, int rank);
    ~TaskInfo();
    /// Check global variables
    bool CheckInputData();
    /// Build various data structures to assist calculation
    bool Build();
    /// Get the number of subbasins in current rank
    int GetSubbasinNumber();
    /// Get the maximum layering ID in current rank
    int GetMaxLayerID() { return max_lyr_; }
    vector<int>& GetRankSubbasinIDs() { return rank_subbsn_id_; }
    map<int, int>& GetSubbasinRank() { return subbsn_rank_; }
    map<int, int>& GetDownstreamID() { return downstream_; }
    map<int, vector<int> >& GetUpstreamIDs() { return upstreams_; }
    map<int, bool>& GetUpstreamsInRank() { return upstreams_inrank_; }
    map<int, vector<int> >& GetLayerSubbasinIDs() { return lyr_subbsns_; }
    map<int, vector<int> >& GetSourceLayerSubbasinIDs() { return srclyr_subbsns_; }
    map<int, vector<int> >& GetNonSourceLayerSubbasinIDs() { return nonsrclyr_subbsns_; }

public:
    int max_len;      ///< Max. subbasins number of all tasks
    int subbsn_count; ///< All subbasins number
    int* subbsn_id;   ///< Subbasin IDs in all groups, length: max_len * size_
    int* lyr_id;      ///< Layering number of each subbasins, length: max_len * size_
    int* down_id;     ///< Down stream subbasin ID of each subbasin, length: max_len * size_
    int* up_count;    ///< Upstream subbasin numbers of each subbasin, length: max_len * size_
    int* up_ids;      ///< Upstream subbasin IDs of each subbasin, length: max_len * size_ * MAX_UPSTREAM

private:
    int size_;                   ///< Number of process
    int rank_;                   ///< Rank ID
    int* subbsn_count_rank_;     ///< Subbasin number in each rank
    int max_lyr_;                ///< Max. layering number of current rank
    vector<int> rank_subbsn_id_; ///< Subbasin IDs in current rank
    /*! Subbasin object -> rank ID (i.e., group ID)
     * Key: Subbasin ID of the whole basin
     * Value: rank ID
     */
    map<int, int> subbsn_rank_;
    /*! Downstream subbasin ID
     * Key: Subbasin ID of the whole basin
     * Value: Downstream subbasin ID
     */
    map<int, int> downstream_;
    /*! Upstream subbasins ID
     * Key: Subbasin ID of the whole basin
     * Value: Upstream subbasins ID
     */
    map<int, vector<int> > upstreams_;
    /*! If upstream subbasins in the same rank
     * Key: Subbasin ID of the whole basin
     * Value: true or false
     */
    map<int, bool> upstreams_inrank_;
    /*! Subbasins of each layer of current rank
     * Key: Layering ID
     * Value: Source subbasin IDs in current rank
     */
    map<int, vector<int> > lyr_subbsns_;
    /*! Source subbasins of each layer of current rank
     * Key: Layering ID
     * Value: Source subbasin IDs in current rank
     */
    map<int, vector<int> > srclyr_subbsns_;
    /*! Non source subbasins in each layer in current slave rank
     * Key: Layering ID
     * Value: Non source subbasin IDs in current rank
     */
    map<int, vector<int> > nonsrclyr_subbsns_;
};
#endif /* SEIMS_MPI_TASK_INFO_H */
