/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
#ifndef SEIMS_POND_CLS_H
#define SEIMS_POND_CLS_H


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
 * \class clsPond
 * \brief Class to store reach related parameters from Pond table
 */

class clsPond: Interface
{
public:
    //! Constructor
    explicit clsPond(const bson_t *&bsonTable);

    //! Get parameters by name
    float clsPond::Get(const string& key);

    //! Set parameters by name
    void Set(const string& key, float value);

	/*~clsPond(void);

	int GetPaddyID() { return PaddyID; }

	int GetPondID1() { return PondID1; }

	int GetPondID2() { return PondID2; }

	int GetPondID3() { return PondID3; }

	int GetReachID() { return ReachID; }*/

private:
    /*!
    * Map container to store parameters
    * key: parameter name
    * value: parameter value
    */
    map<string, float> param_map_;

	////! paddy id, which need to irrigation
	//int PaddyID;

	////! pond id, which used to irrigate the paddy related first
	//int PondID1;

	////! pond id, which used to irrigate the paddy related second
	//int PondID2;

	////! pond id, which used to irrigate the paddy related third
	//int PondID3;

	////! reach id, which used to irrigate the paddy if the three pond are not enough
	//int ReachID;
};

class clsPonds: Interface
{
public:
	/*!
     * \brief Constructor
     *
     * Query pond table from MongoDB
     *
     * \param[in] conn mongoc client instance
     * \param[in] dbName Database name
     * \param[in] collectionName Pond collection name
     */
    clsPonds(MongoClient* conn, const string& db_name, const string& collection_name, LayeringMethod mtd = UP_DOWN);

	/// Destructor
	~clsPonds();

	/// Get pond number
    int GetPaddyNumber() const { return this->paddy_num_; }

	/// Get single pond information by paddy ID
	clsPond *GetPondByID(int id) {
        if (pondsInfo_.find(id) != pondsInfo_.end()) {
            return pondsInfo_.at(id);
        }
        return nullptr;
        //return m_pondsInfo.at(id); 
    }

	/// Get paddy IDs (vector)
    vector<int>& GetPaddyIDs() { return this->paddyIDs_; }

    /*!
    * \brief Update reach/channel parameters according to calibration settings
    */
    void Update(map<string, ParamInfo *>& caliparams_map);

private:
	/// paddy number
    int paddy_num_;
	/// paddy IDs
    vector<int> paddyIDs_;
    /* Map container to store all ponds information
     * key: paddy ID
     * value: clsReach instance (pointer)
     */
    map<int, clsPond *> pondsInfo_;
};
#endif