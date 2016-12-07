/*!
 * \file clsReach.h
 * \brief Class to store reach related parameters from REACHES table
 *
 * \author LiangJun Zhu
 * \version 1.0
 * \date May. 2016
 *
 */
#pragma once

#include <string>
#include "MetadataInfoConst.h"
#include "mongoc.h"
#include "MongoUtil.h"
#include "ModelException.h"
#include <map>

using namespace std;

/*!
* \ingroup data
 * \class clsReach
 * \brief Class to store reach related parameters from REACHES table
 */
class clsReach
{
public:
    //! Constructor
    clsReach(const bson_t *&bsonTab);

    //! Destructor
    ~clsReach(void);

    //! Reset the contents of the object to default values
    void Reset(void);

    int GetSubbasinID() { return SubbasinID; }

    float GetArea() { return this->Area; }

    float GetDepth() { return this->Depth; }

    int GetDownStream() { return DownStream; }

    int GetDownUpOrder() { return DownUpOrder; }

    int GetUpDownOrder() { return UpDownOrder; }

    int GetGroup() { return Group; }

    int GetGroupDivided() { return GroupDivided; }

    float GetLength() { return this->Length; }

    float GetManning() { return this->Manning; }

    int GetNumCells() { return this->NumCells; }

    float GetSlope() { return this->Slope; }

    float GetV0() { return this->V0; }

    float GetWidth() { return this->Width; }

	float GetSideSlope() {return this->SideSlope;}

    float GetBc1() { return this->bc1; }

    float GetBc2() { return this->bc2; }

    float GetBc3() { return this->bc3; }

    float GetBc4() { return this->bc4; }

    float GetRs1() { return this->rs1; }

    float GetRs2() { return this->rs2; }

    float GetRs3() { return this->rs3; }

    float GetRs4() { return this->rs4; }

    float GetRs5() { return this->rs5; }

    float GetRk1() { return this->rk1; }

    float GetRk2() { return this->rk2; }

    float GetRk3() { return this->rk3; }

    float GetRk4() { return this->rk4; }

	float GetCover(){return this->cover;}

	float GetErod(){return this->erod;}

	float GetDisOxygen(){return this->disox;}

	float GetCOD(){return this->cod;}

	float GetAlgae(){return this->algae;}

	float GetOrgN(){return this->orgn;}

	float GetNH4(){return this->nh4;}

	float GetNO2(){return this->no2;}

	float GetNO3(){return this->no3;}

	float GetOrgP(){return this->orgp;}

	float GetSolP(){return this->solp;}

	float GetGWNO3(){return this->gwno3;}

	float GetGWSolP(){return this->gwsolp;}
private:
    //! Subbasin area
    float Area;
    //! Depth
    float Depth;
    //! Downstream reach index
    int DownStream;
    //! DOWN_UP stream order
    int DownUpOrder;
    //! Group Index
    int Group;
    //! Group divided
    int GroupDivided;
    //! Length
    float Length;
    //! Manning coefficient
    float Manning;
    //! Cell numbers
    int NumCells;
    //! Slope gradient
    float Slope;
    //! Subbasin ID
    int SubbasinID;
    //! UP_DOWN stream order
    int UpDownOrder;
    //! V0
    float V0;
    //! Width
    float Width;
	/// inverse of the channel side slope
	float SideSlope;
    /// rate constant for biological oxidation of NH3 to NO2 in reach at 20 deg C
    float bc1;
    /// rate constant for biological oxidation of NO2 to NO3 in reach at 20 deg C
    float bc2;
    /// rate constant for biological oxidation of organic N to ammonia in reach at 20 deg C
    float bc3;
    /// rate constant for biological oxidation of organic P to dissolved P in reach at 20 deg C
    float bc4;
    /// local algal settling rate in reach at 20 deg C (m/day)
    float rs1;
    /// benthos source rate for dissolved phosphorus in reach at 20 deg C (mg disP-P)/((m**2)*day)
    float rs2;
    /// benthos source rate for ammonia nitrogen in reach at 20 deg C (mg NH4-N)/((m**2)*day)
    float rs3;
    /// rate coefficient for organic nitrogen settling in reach at 20 deg C (1/day)
    float rs4;
    /// organic phosphorus settling rate in reach at 20 deg C (1/day)
    float rs5;
    /// CBOD doxygenation rate coefficient in reach at 20 deg C (1/day)
    float rk1;
    /// reaeration rate in accordance with Fickian diffusion in reach at 20 deg C (1/day)
    float rk2;
    /// rate of loss of CBOD due to settling in reach at 20 deg C (1/day)
    float rk3;
    /// sediment oxygen demand rate in reach at 20 deg C (mg O2/ ((m**2)*day))
    float rk4;
	/// erosion related
	/// cover factoer
	float cover;
	/// erodibility factor
	float erod;
	/// Concentration of nitrate in groundwater contribution to streamflow from subbasin (mg N/l).
	float gwno3;
	/// Concentration of soluble phosphorus in groundwater contribution to streamflow from subbasin (mg P/l).
	float gwsolp;
	/// Initial dissolved oxygen concentration in the reach.	[mg /l]
	float disox;
	/// Initial biochemical oxygen demand in the reach .	[mg /l]. note that, this is different with cbod in SWAT
	float cod;
	/// Initial chlorophyll-a concentration in the reach.	[mg /l]
	float algae;
	/// Initial organic nitrogen concentration in the reach.	[mg /l]
	float orgn;
	/// Initial ammonia concentration in the reach.	[mg /l]
	float nh4;
	/// Initial nitrite concentration in the reach
	float no2;
	/// Initial nitrate concentration in the reach
	float no3;
	/// Initial organic phosphorus concentration in the reach.	[mg /l]
	float orgp;
	/// Initial dissolved phosphorus concentration in the reach.	[mg /l]
	float solp;
};

/*!
 * \class clsReaches
 * \ingroup data
 *
 * \brief Read and store all reaches information as input parameters
 *
 */
class clsReaches
{
public:
    /*!
     * \brief Constructor
     *
     * Query reach table from MongoDB
     *
     * \param[in] conn mongoc client instance
     * \param[in] dbName Database name
     * \param[in] collectionName Reach collection name
     */
    clsReaches(mongoc_client_t *conn, string &dbName, string collectionName);

    /// Destructor
    ~clsReaches();

    /// Get single reach information by subbasin ID
    clsReach *GetReachByID(int id) { return m_reachesInfo.at(id); }

    /// Get reach number
    int GetReachNumber() { return this->m_reachNum; }

    /// Get reach IDs (vector)
    vector<int>& GetReachIDs() { return this->m_reachIDs; }

private:
    /// reaches number
    int m_reachNum;
    /// reach IDs
    vector<int> m_reachIDs;
    /* Map container to store all reaches information
     * key: reach ID
     * value: clsReach instance (pointer)
     */
    map<int, clsReach *> m_reachesInfo;
};