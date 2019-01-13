/*!
 * \file clsReach.h
 * \brief Class to store reach related parameters from REACHES table
 *
 * Changelog:
 *   - 1. 2017-05-30 - lj - Update MongoDB functions.
 *                          Get 1D arrays of reach properties to keep synchronization among modules.
 *   - 2. 2017-12-26 - lj - Code refactor.
 *
 * \author LiangJun Zhu
 * \version 1.1
 */
#ifndef SEIMS_REACH_CLS_H
#define SEIMS_REACH_CLS_H

#include <map>

#include "basic.h"
#include "db_mongoc.h"

#include "seims.h"
#include "ParamInfo.h"

using namespace ccgl;
using namespace db_mongoc;
using std::map;

/*!
 * \ingroup data
 * \class clsReach
 * \brief Class to store reach related parameters from REACHES table
 */
class clsReach: Interface {
public:
    //! Constructor
    explicit clsReach(const bson_t*& bson_table);

    //! Get parameters by name
    float Get(const string& key);

    //! Get group index
    int GetGroupIndex(const string& method, int size);

    //! Set parameters by name
    void Set(const string& key, float value);

    /*!
    * \brief Calculate derived parameters after updating the input parameters.
    */
    void DerivedParameters();

private:
    /*!
     * Map container to store parameters
     * key: parameter name
     * value: parameter value
     */
    map<string, float> param_map_;
    /*!
     * Group numbers, e.g., [1, 2, 3, 8, 16]
     */
    vector<int> group_number_;
    /*!
     * Group index if each group number and group method, e.g.,
     * {'KMETIS': {1: 0, 2: 1, 3: 1, 8: 2, 16: 15}, 'PMETIS': {...}}
     */
    map<string, map<int, int> > group_index_;
};

/*!
 * \class clsReaches
 * \ingroup data
 *
 * \brief Read and store all reaches information as input parameters
 *
 */
class clsReaches: Interface {
public:
    /*!
     * \brief Constructor, query reach table from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] db_name Database name
     * \param[in] collection_name Reach collection name
     * \param[in] mtd layering method, the default is UP_DOWN, \sa LayeringMethod
     */
    clsReaches(MongoClient* conn, const string& db_name, const string& collection_name, LayeringMethod mtd = UP_DOWN);

    /// Destructor
    ~clsReaches();

    /// Get single reach information by subbasin ID (1 ~ N)
    clsReach* GetReachByID(const int id) {
        if (reaches_obj_.find(id) != reaches_obj_.end()) {
            return reaches_obj_.at(id);
        }
        return nullptr;
    }

    /// Get reach number
    int GetReachNumber() const { return this->reach_num_; }

    /*!
     * \brief Get 1D array of reach property
     * \param[in] key
     * \param[out] data 1D array with length of N+1, the first element is Reach number.
     */
    void GetReachesSingleProperty(const string& key, float** data);

    /// Get upstream IDs
    vector<vector<int> >& GetUpStreamIDs() { return reach_up_streams_; }

    /// Get downstream ID
    map<int, int>& GetDownStreamID() { return reach_down_stream_; }

    /// Get map of reach layers
    map<int, vector<int> >& GetReachLayers() { return reach_layers_; };

    /*!
     * \brief Update reach/channel parameters according to calibration settings
     */
    void Update(map<string, ParamInfo *>& caliparams_map);

private:
    /// reaches number
    int reach_num_;
    /*!
     * Upstream Ids (The value is -1 if there if no upstream reach)
     * reach_up_streams_.size() = N+1
     * reach_up_streams_[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > reach_up_streams_;
    /*!
     * Downstream ID, -1 indicates no downstream, i.e., the outlet reach
     */
    map<int, int> reach_down_stream_;
    /*!
     * Reach layers according to \a LayeringMethod
     */
    map<int, vector<int> > reach_layers_;
    /*!
     * Map container to store all reaches information
     * key: reach ID, 1 ~ N
     * value: clsReach instance (pointer)
     */
    map<int, clsReach *> reaches_obj_;

    /*! Map of all reaches properties arranged as 1D array
     * the first value is reach number
     */
    map<string, float *> reaches_properties_;
};
#endif /* SEIMS_REACH_CLS_H */
