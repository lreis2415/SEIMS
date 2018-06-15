/*!
 * \brief Simulates depressional areas that do not drain to the stream network (pothole) and impounded areas such as rice paddies
 * \author Liang-Jun Zhu
 * \date Sep 2016
 *           1. Source code of SWAT include: pothole.f
 *           2. Add the simulation of Ammonia n transported with surface runoff, 2016-9-27
 *           3. Add m_depEvapor and m_depStorage from DEP_LENSLEY module
 *           4. Using a simple model (first-order kinetics equation) to simulate N transformation in impounded area.
 * \data 2016-10-10
 * \description: 1. Update all related variables after the simulation of pothole.
 */

#include "SimulationModule.h"

class POND : public SimulationModule
{
private:
	/// pond number
	int m_npond;
	/// pond id
	float *m_pond;
	/// pond volumn
	float *m_pondVol;
	/// max pond depth
	float *m_pondVolMax;
	/// pond surface
	float *m_pondSurfaceArea;
	///
	float m_evap_coe;
	/// hydraulic conductivity of soil surface of pond
	float m_pond_k;
	///
	float *m_pondCellEvap;
	///
	float *m_pondCellSeep;
	/// conversion factor (mm/ha => m^3)
	float m_cnv; 
    /// valid cells number
    int m_nCells;	
	/// cell width, m
	float m_cellWidth;
	/// cell area, ha
	float m_cellArea;
	/// landuse code
	float *m_landuse;
	/// all pond id
	vector<int> m_pondIds;
	/*
	 * pond id cell
	 * key: pond id
	 * value: pond cell id
	 */
    map<int, vector<int> > m_pondIdInfo;
	/// paddy number
	int m_paddyNum;
	/// paddy IDs
	vector<int> m_paddyIDs;
	/// pond id, which used to irrigate the paddy related first
	float *m_pondID1;
	/// pond id, which used to irrigate the paddy related second
	float *m_pondID2;
	/// pond id, which used to irrigate the paddy related third
	float *m_pondID3;
	/// reach id, which used to irrigate the paddy if the three pond are not enough
	float *m_reachID;
	/// the irrigate depth
	float *m_irrDepth;
	/// 
	float m_embnkfr_pr;
	/// pet
	float *m_pet;
	/// saturated conductivity
	float **m_ks;
	/// amount of water stored in soil layers on current day
	float **m_soilStorage;
	/// reach storage (m^3) at time, t
	float *m_chStorage;
	/* Map container to store all ponds information
     * key: paddy ID
     * value: clsReach instance (pointer)
     */
    map<int, clsPond *> m_pondsInfo;
	/// reaches number
	int m_nReaches;
	/// maximum soil layers, mlyr in SWAT
	int m_soilLayers;
	/**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
	/// surface runoff (mm)
	float *m_surfaceRunoff;
	/// depth of grid cell in pond 
	float *m_pondCellVol;
	/// excess precipitation calculated in the infiltration module
	float *m_pe;
	/// flow out index
	float *m_flowOutIndex;
	/* Map container to store all ponds flow out information
     * key: paddy ID
     * value: down stream pond
     */
	map<int, vector<int> > m_pondDownPond;
	/* Map container to store all ponds flow in information
     * key: pond cell id
     * value: flow in cell id
     */
	map<int, vector<int> > m_pondFlowInCell;
	/// sediment yield transported on each cell, kg
	float *m_sedYield;
	//! sand yield
	float *m_sandYield;
	//! silt yield
	float *m_siltYield;
	//! clay yield
	float *m_clayYield;
	//! small aggregate yield
	float *m_smaggreYield;
	//! large aggregate yield
	float *m_lgaggreYield;
	/// sediment amount kg
	float *m_pondSed;
	/// sand 
	float *m_pondSand;
	/// silt
	float *m_pondSilt;
	/// clay
	float *m_pondClay;
	/// small aggregate
	float *m_pondSag;
	/// large aggregate
	float *m_pondLag;
	/// no3 amount kg
	float *m_pondNo3;
	/// nh4 amount kg
	float *m_pondNH4;
	/// orgN amount kg
	float *m_pondOrgN;
	/// soluble phosphorus amount, kg
	float *m_pondSolP;
	/// orgP amount kg
	float *m_pondOrgP;
	/// active mineral P kg
	float *m_pondActMinP;
	/// stable mineral P kg
	float *m_pondStaMinP;
	/// amount of nitrate transported with surface runoff, kg/ha
	float *m_surqNo3;
	/// amount of ammonian transported with surface runoff, kg/ha
	float *m_surqNH4;
	/// amount of soluble phosphorus transported with surface runoff, kg/ha
	float *m_surqSolP;
	/// , kg/ha
	float *m_surqCOD;
	/// , kg/ha
	float *m_sedOrgN;
	///, kg/ha
	float *m_sedOrgP;
	/// , kg/ha
	float *m_sedActiveMinP;
	/// , kg/ha
	float *m_sedStableMinP;
	/// Soluble phosphorus decay rate in impounded water body
	float m_pondSolPDecay;
	/// volatilization rate constant in impounded water body, /day
	float m_kVolat;
	/// nitrification rate constant in impounded water body, /day
	float m_kNitri;
	/// time step (second)
	float m_timestep;
	 
public:
    //! Constructor
    POND(void);

    //! Destructor
    ~POND(void);

    virtual int Execute();

	virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int n, int col, float **data);

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);
	/*!
     * \brief check the input size of 2D data. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The first dimension input data 
	 * \param[in] col The second dimension of input data 
     * \return bool The validity of the dimension
     */
    bool CheckInputSize2D(const char *key, int n, int col);
	/// initialize all possible outputs
	void initialOutputs();
	
	void POND::pondSurfaceArea(int id);
	
	void POND::pondSimulate(int id, int cellId);
	
	void POND::findFlowInCell(int id, int cellId);

	int POND::findFlowOutPond(int id, int cellId);

	void POND::pondRelease(int id, float qOut);
};
