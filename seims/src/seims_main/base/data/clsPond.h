/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
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

class clsPond
{
public:
	clsPond(const bson_t *&bsonTable);

	~clsPond(void);

	int GetPaddyID() { return PaddyID; }

	int GetPondID1() { return PondID1; }

	int GetPondID2() { return PondID2; }

	int GetPondID3() { return PondID3; }

	int GetReachID() { return ReachID; }

private:
	//! paddy id, which need to irrigation
	int PaddyID;

	//! pond id, which used to irrigate the paddy related first
	int PondID1;

	//! pond id, which used to irrigate the paddy related second
	int PondID2;

	//! pond id, which used to irrigate the paddy related third
	int PondID3;

	//! reach id, which used to irrigate the paddy if the three pond are not enough
	int ReachID;
};

class clsPonds
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
	clsPonds(mongoc_client_t *conn, string &dbName, string collectionName);

	/// Destructor
	~clsPonds();

	/// Get pond number
	int GetPaddyNumber() { return this->m_paddyNum; }

	/// Get single pond information by paddy ID
	clsPond *GetPondByID(int id) { return m_pondsInfo.at(id); }

	/// Get paddy IDs (vector)
	vector<int>& GetPaddyIDs() { return this->m_paddyIDs; }

private:
	/// paddy number
	int m_paddyNum;
	/// paddy IDs
    vector<int> m_paddyIDs;
    /* Map container to store all ponds information
     * key: paddy ID
     * value: clsReach instance (pointer)
     */
    map<int, clsPond *> m_pondsInfo;
};