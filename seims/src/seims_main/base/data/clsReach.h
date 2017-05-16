/*!
 * \brief Class to store reach related parameters from REACHES table
 * \author LiangJun Zhu
 * \version 1.1
 * \date May. 2017
 * \revised LJ - Update MongoDB functions
 *             - Get 1D arrays of reach properties to keep synchronization among modules
 */
#ifndef SEIMS_REACH_CLS_H
#define SEIMS_REACH_CLS_H

#include "text.h"
#include "utilities.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \enum ReachAttr
 * \TODO Find a more elegant way to read, store, and get reaches attributes.
 */
enum ReachAttr
{
    subbasinid,
    num_cells,
    group,
    group_divide,
    kmetis,
    pmetis,
    downstream,
    up_down_order,
    down_up_order,
    width,
    side_slope,
    length,
    depth,
    v0,
    area,
    manning,
    slope,
    bc1,
    bc2,
    bc3,
    bc4,
    rs1,
    rs2,
    rs3,
    rs4,
    rs5,
    rk1,
    rk2,
    rk3,
    rk4,
    cover,
    erod,
    disox,
    bod,
    algae,
    orgn,
    nh4,
    no2,
    no3,
    orgp,
    solp,
    gwno3,
    gwsolp,
    attrcount
};
/*!
 * \ingroup data
 * \class clsReach
 * \brief Class to store reach related parameters from REACHES table
 */
class clsReach {
public:
    //! Constructor
    clsReach(const bson_t *&bsonTab);

    //! Destructor
    ~clsReach(void);

    //! Reset the contents of the object to default values
    void Reset(void);

    int GetSubbasinID(void) { return this->SubbasinID; }
    float GetArea(void) { return this->Area; }
    float GetDepth(void) { return this->Depth; }
    int GetDownStream(void) { return this->DownStream; }
    int GetDownUpOrder(void) { return this->DownUpOrder; }
    int GetUpDownOrder(void) { return this->UpDownOrder; }
    int GetGroup(void) { return this->Group; }
    int GetGroupDivided(void) { return this->GroupDivided; }
    float GetLength(void) { return this->Length; }
    float GetManning(void) { return this->Manning; }
    int GetNumCells(void) { return this->NumCells; }
    float GetSlope(void) { return this->Slope; }
    float GetV0(void) { return this->V0; }
    float GetWidth(void) { return this->Width; }
    float GetSideSlope(void) { return this->SideSlope; }
    float GetBc1(void) { return this->bc1; }
    float GetBc2(void) { return this->bc2; }
    float GetBc3(void) { return this->bc3; }
    float GetBc4(void) { return this->bc4; }
    float GetRs1(void) { return this->rs1; }
    float GetRs2(void) { return this->rs2; }
    float GetRs3(void) { return this->rs3; }
    float GetRs4(void) { return this->rs4; }
    float GetRs5(void) { return this->rs5; }
    float GetRk1(void) { return this->rk1; }
    float GetRk2(void) { return this->rk2; }
    float GetRk3(void) { return this->rk3; }
    float GetRk4(void) { return this->rk4; }
    float GetCover(void) { return this->cover; }
    float GetErod(void) { return this->erod; }
    float GetDisOxygen(void) { return this->disox; }
    float GetCOD(void) { return this->cod; }
    float GetAlgae(void) { return this->algae; }
    float GetOrgN(void) { return this->orgn; }
    float GetNH4(void) { return this->nh4; }
    float GetNO2(void) { return this->no2; }
    float GetNO3(void) { return this->no3; }
    float GetOrgP(void) { return this->orgp; }
    float GetSolP(void) { return this->solp; }
    float GetGWNO3(void) { return this->gwno3; }
    float GetGWSolP(void) { return this->gwsolp; }

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
    /// Initial dissolved oxygen concentration in the reach. [mg /l]
    float disox;
    /// Initial biochemical oxygen demand in the reach. [mg /l]. note that, this is different with cbod in SWAT
    float cod;
    /// Initial chlorophyll-a concentration in the reach. [mg /l]
    float algae;
    /// Initial organic nitrogen concentration in the reach. [mg /l]
    float orgn;
    /// Initial ammonia concentration in the reach. [mg /l]
    float nh4;
    /// Initial nitrite concentration in the reach
    float no2;
    /// Initial nitrate concentration in the reach
    float no3;
    /// Initial organic phosphorus concentration in the reach. [mg /l]
    float orgp;
    /// Initial dissolved phosphorus concentration in the reach. [mg /l]
    float solp;
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
    clsReaches(MongoClient* conn, string &dbName, string collectionName);

    /// Destructor
    ~clsReaches(void);

    /// Get single reach information by subbasin ID
    clsReach *GetReachByID(int id) { return m_reachesMap.at(id); }

    /// Get reach number
    int GetReachNumber(void) const { return this->m_reachNum; }

    /// Get reach IDs (vector)
    vector<int>& GetReachIDs(void) { return this->m_reachIDs; }

    /*!
     * \brief Get 1D array of reach property
     * \TODO Implement later. LJ
     */
    void GetReachesSingleProperty(const char *key, float **data) {};

private:
    /// reaches number
    int m_reachNum;
    /// reach IDs
    vector<int> m_reachIDs;
    /*!
     * Map container to store all reaches information
     * key: reach ID
     * value: clsReach instance (pointer)
     */
    map<int, clsReach *> m_reachesMap;

    /*! 2D array to store all reaches properties
     * Row index is the index of reach property
     * Col index is the index of m_reachIDs
     */
    float** m_reachesProperties;
};
#endif /* SEIMS_REACH_CLS_H */
