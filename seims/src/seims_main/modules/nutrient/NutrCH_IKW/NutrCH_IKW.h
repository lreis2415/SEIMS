
/*!
 * \file NutrCH_IKW.h
 * \brief Calculates in-stream nutrient transformations.
 * \author Huiran Gao
 * \date Jun 2016
 */

#pragma once
#ifndef SEIMS_NutCHRout_PARAMS_INCLUDE
#define SEIMS_NutCHRout_PARAMS_INCLUDE

#include <string>
#include <map>
#include <vector>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup NutCHRout
 * \ingroup Nutrient
 * \brief Calculates in-stream nutrient transformations.
 */

/*!
 * \class NutrCH_IKW
 * \ingroup NutCHRout
 *
 * \brief Calculates the concentration of nutrient in reach.
 *
 */

struct MuskWeights
{
    float c1;
    float c2;
    float c3;
    float c4;
    float dt;
    int n;  ///< number of division of the origin time step
};

class NutrientCH_IKW : public SimulationModule
{
public:
    NutrientCH_IKW(void);

    ~NutrientCH_IKW(void);
    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

    virtual int Execute();

    virtual void GetValue(const char *key, float *value);

    //virtual void Get1DData(const char *key, int *n, float **data);
    virtual void Get2DData(const char* key, int* nRows, int* nCols, float*** data);
private:

	/// cell width of grid map (m)
	float m_CellWith;
	/// number of valid cells
	int m_nCells;

    /// time step (hr)
    int m_dt;
	/// stream order
	float *m_streamOrder;
    /// downstream id (The value is 0 if there if no downstream reach)
	float *m_reachDownStream;
	/// stream link
	float *m_streamLink;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;
	/**
    *	@brief reach links
    *	key: index of the reach
    *	value: vector of cell index
    */
    map<int, vector<int> > m_reachs;
    /// id the reaches
    float *m_reachId;
    /// reaches number
    int m_nReaches;
    map<int, vector<int> > m_reachLayers;
    /// Reach information
    clsReaches *m_reaches;
	/// id of source cells of reaches
	int *m_sourceCellIds;
	/// channel number
	int m_chNumber;
	/**
    *	\brief 2d array of flow in cells
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
    /// flow out index
	float *m_flowOutIndex;
	/// map from subbasin id to index of the array
	map<int, int> m_idToIndex;

    /// input data
    /// bank flow recession constant
    float m_aBank;
    float m_qUpReach;

    float m_ai0;    /// ratio of chlorophyll-a to algal biomass (ug chla/mg alg)
    float m_ai1;    /// fraction of algal biomass that is nitrogen (mg N/mg alg)
    float m_ai2;    /// fraction of algal biomass that is phosphorus (mg P/mg alg)
    float m_ai3;    /// the rate of oxygen production per unit of algal photosynthesis (mg O2/mg alg)
    float m_ai4;    /// the rate of oxygen uptake per unit of algae respiration (mg O2/mg alg)
    float m_ai5;    /// the rate of oxygen uptake per unit of NH3 nitrogen oxidation (mg O2/mg N)
    float m_ai6;    /// the rate of oxygen uptake per unit of NO2 nitrogen oxidation (mg O2/mg N)

    float m_lambda0;     /// non-algal portion of the light extinction coefficient
    float m_lambda1;     /// linear algal self-shading coefficient
    float m_lambda2;     /// nonlinear algal self-shading coefficient

    float m_k_l;        /// half saturation coefficient for light (MJ/(m2*hr))
    float m_k_n;        /// half saturation constant for nitrogen (mg N/L)
    float m_k_p;        /// half saturation constant for phosphorus (mg P/L)
    /// algal preference factor for ammonia
    float m_p_n;
    /// fraction of solar radiation computed in the temperature heat balance that is photo synthetically active
    float tfact;
    /// fraction of overland flow
    float m_rnum1;
    /// option for calculating the local specific growth rate of algae
    //     1: multiplicative:     u = mumax * fll * fnn * fpp
    //     2: limiting nutrient: u = mumax * fll * Min(fnn, fpp)
    //     3: harmonic mean: u = mumax * fll * 2. / ((1/fnn)+(1/fpp))
    int igropt;
    /// maximum specific algal growth rate at 20 deg C
    float m_mumax;
    /// algal respiration rate at 20 deg C (1/day)
    float m_rhoq;

    /// day length for current day (h)
    float *m_daylen;
    /// solar radiation for the day (MJ/m2)
    float *m_sra;
    float *m_bankStorage;
    /// overland flow to streams from each subbasin (m3/s)
    float *m_qsSub;
    /// inter-flow to streams from each subbasin (m3/s)
    float *m_qiSub;
    /// groundwater flow out of the subbasin (m3/s)
    float *m_qgSub;

    float *m_chOrder;
    float *m_chWidth;
    float *m_chDepth;

    /// channel outflow
    float *m_qsCh;
    float *m_qiCh;
    float *m_qgCh;
    /// reach storage (m3) at time t
    float *m_chStorage;
    /// channel water depth m
    float *m_chWTdepth;
    /// temperature of water in reach (deg C)
    float *m_wattemp;

    float *m_bc1;        /// rate constant for biological oxidation of NH3 to NO2 in reach at 20 deg C
    float *m_bc2;        /// rate constant for biological oxidation of NO2 to NO3 in reach at 20 deg C
    float *m_bc3;        /// rate constant for biological oxidation of organic N to ammonia in reach at 20 deg C
    float *m_bc4;        /// rate constant for biological oxidation of organic P to dissolved P in reach at 20 deg C

    float *m_rs1;        /// local algal settling rate in reach at 20 deg C (m/day)
    float *m_rs2;        /// benthos source rate for dissolved phosphorus in reach at 20 deg C (mg disP-P)/((m**2)*day)
    float *m_rs3;        /// benthos source rate for ammonia nitrogen in reach at 20 deg C (mg NH4-N)/((m**2)*day)
    float *m_rs4;        /// rate coefficient for organic nitrogen settling in reach at 20 deg C (1/day)
    float *m_rs5;        /// organic phosphorus settling rate in reach at 20 deg C (1/day)

    float *m_rk1;      /// CBOD deoxygenation rate coefficient in reach at 20 deg C (1/day)
    float *m_rk2;      /// reaeration rate in accordance with Fickian diffusion in reach at 20 deg C (1/day)
    float *m_rk3;      /// rate of loss of CBOD due to settling in reach at 20 deg C (1/day)
    float *m_rk4;      /// sediment oxygen demand rate in reach at 20 deg C (mg O2/ ((m**2)*day))

    /// amount of nitrate transported with lateral flow
    float *m_latno3ToCh;
    /// amount of nitrate transported with surface runoff
    float *m_surqno3ToCh;
    /// amount of soluble phosphorus in surface runoff
    float *m_surqsolpToCh;
    /// nitrate loading to reach in groundwater
    float *m_no3gwToCh;
    /// soluble P loading to reach in groundwater
    float *m_minpgwToCh;
    // amount of organic nitrogen in surface runoff
    float *m_sedorgnToCh;
    // amount of organic phosphorus in surface runoff
    float *m_sedorgpToCh;
    // amount of active mineral phosphorus absorbed to sediment in surface runoff
    float *m_sedminpaToCh;
    // amount of stable mineral phosphorus absorbed to sediment in surface runoff
    float *m_sedminpsToCh;
    /// amount of ammonium transported with lateral flow
    float *m_ammoToCh;
    /// amount of nitrite transported with lateral flow
    float *m_nitriteToCh;
    /// cod to reach in surface runoff (kg)
    float *m_codToCh;

    //float m_vScalingFactor;
    /// for muskingum
    //float m_x;
    //float m_co1;

    /// output data
    /// algal biomass concentration in reach (mg/L)
    float **m_algae;
    /// organic nitrogen concentration in reach (mg/L)
    float **m_organicn;
    /// organic phosphorus concentration in reach (mg/L)
    float **m_organicp;
    /// ammonia concentration in reach (mg/L)
    float **m_ammonian;
    /// nitrite concentration in reach (mg/L)
    float **m_nitriten;
    /// nitrate concentration in reach (mg/L)
    float **m_nitraten;
    /// dissolved phosphorus concentration in reach (mg/L)
    float **m_disolvp;
    /// carbonaceous oxygen demand in reach (mg/L)
    float **m_rch_cod;
    /// dissolved oxygen concentration in reach (mg/L)
    float **m_rch_dox;
    /// chlorophyll-a concentration in reach (mg chl-a/L)
    float **m_chlora;
    // saturation concentration of dissolved oxygen (mg/L)
    float m_soxy;

	// Outlet

private:

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);

    /*!
    * \brief In-stream nutrient transformations and water quality calculations.
	*
	* \param iReach The index in the array of m_reaches
	* \param iCell The index of cell array in each reach
	* \param ID The id of cell in grid map
	*
    * \return void
    */
    void NutrientinChannel(int iReach, int iCell, int id);

    /*!
    * \brief Corrects rate constants for temperature.
     *
     *    r20         1/day         value of the reaction rate coefficient at the standard temperature (20 degrees C)
     *    thk         none          temperature adjustment factor (empirical constant for each reaction coefficient)
     *    tmp         deg C         temperature on current day
     *
     * \return float
     */
    float corTempc(float r20, float thk, float tmp);

    void initialOutputs();

	float GetNutrInFlow(int iReach, int iCell, int id, float* Nutr, float** CHNutr);

    //void GetCoefficients(float reachLength, float v0, MuskWeights& weights);
};

#endif





