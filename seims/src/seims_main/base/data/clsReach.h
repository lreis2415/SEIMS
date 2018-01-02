/*!
 * \brief Class to store reach related parameters from REACHES table
 * \author LiangJun Zhu
 * \version 1.1
 * \date May. 2017
 * \revised LJ - Update MongoDB functions
 *             - Get 1D arrays of reach properties to keep synchronization among modules
 *             - Code refactor. 2017-12-26
 */
#ifndef SEIMS_REACH_CLS_H
#define SEIMS_REACH_CLS_H

#include "seims.h"
#include "ParamInfo.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \class clsReach
 * \brief Class to store reach related parameters from REACHES table
 */
class clsReach {
public:
    //! Constructor
    explicit clsReach(const bson_t *&bsonTab);

    //! Destructor
    ~clsReach() = default;

    //! Get parameters by name
    float Get(const string &key);

    //! Set parameters by name
    void Set(const string &key, float value);

private:
    /*!
     * Map container to store parameters
     * key: parameter name
     * value: parameter value
     */
    map<string, float> m_paramMap;
};

/*!
 * \class clsReaches
 * \ingroup data
 *
 * \brief Read and store all reaches information as input parameters
 *
 */
class clsReaches {
public:
    /*!
     * \brief Constructor, query reach table from MongoDB
     * \param[in] conn MongoClient instance
     * \param[in] dbName Database name
     * \param[in] collectionName Reach collection name
     */
    clsReaches(MongoClient *conn, string &dbName, string collectionName);

    /// Destructor
    ~clsReaches();

    /// Get single reach information by subbasin ID (1 ~ N)
    inline clsReach *GetReachByID(int id) {
        if (m_reachesMap.find(id) != m_reachesMap.end()) { return m_reachesMap.at(id); }
        else { return nullptr; }
    }

    /// Get reach number
    int GetReachNumber() const { return this->m_reachNum; }

    /*!
     * \brief Get 1D array of reach property
     * \param[out[ data, 1D array with length of N+1, the first element is Reach number.
     */
    void GetReachesSingleProperty(string key, float **data);

    /// Get upstream IDs
    vector<vector<int> > GetUpStreamIDs() const { return m_reachUpStream; }

    /// Get map of reach layers
    map<int, vector<int> > GetReachLayers(LayeringMethod mtd = UP_DOWN);

    /*!
     * \brief Update reach/channel parameters according to calibration settings
     */
    void Update(const map<string, ParamInfo *> &caliparams_map);

private:
    /// reaches number
    int m_reachNum;
    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;
    /*!
     * Reach layers according to \a LayeringMethod
     */
    map<int, vector<int> > m_reachLayers;
    /*!
     * Map container to store all reaches information
     * key: reach ID, 1 ~ N
     * value: clsReach instance (pointer)
     */
    map<int, clsReach *> m_reachesMap;

    /*! Map of all reaches properties arranged as 1D array
     * the first value is reach number
     */
    map<string, float *> m_reachesPropMap;
};
#endif /* SEIMS_REACH_CLS_H */
