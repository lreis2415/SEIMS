/*!
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637, plantmod.f
 * \author LiangJun Zhu
 * \date June 2016
 *
 * \review LiangJun Zhu
 * \date 2016-10-7
 * \description:  1. add some code of CENTURY calculation
 */

#pragma once

#include <string>
#include <ctime>
#include "api.h"
#include "PlantGrowthCommon.h"
#include "ClimateParams.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup BIO_EPIC
 * \ingroup Ecology
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637
 */
/*!
 * \class Biomass_EPIC
 * \ingroup BIO_EPIC
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637
 */
class Biomass_EPIC : public SimulationModule
{
public:
    //! Constructor
    Biomass_EPIC(void);

    //! Destructor
    ~Biomass_EPIC(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Set2DData(const char *key, int nRows, int nCols, float **data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputSize2D(const char *key, int n, int col);

    bool CheckInputData(void);

private:
    /// valid cells number
    int m_nCells;
    /// years of climate data
    int m_nClimDataYrs;
    /**  climate inputs  **/

    /// CO2 concentration
    float m_co2;
    /// mean air temperature
    float *m_tMean;
    /// min air temperature
    float *m_tMin;
    /// solar radiation
    float *m_SR;
    /// average annual air temperature
    float *m_tMeanAnn;
    /// minimum day length
    float *m_dayLenMin;
    /// dormancy threshold
    float *m_dormHr;
    /**  soil properties  **/

    /// soil layers
    float *m_nSoilLayers;
    /// maximum soil layers
    int m_soilLayers;
    /// maximum root depth
    float *m_soilZMX;
    /// albedo when soil is moist
    float *m_soilALB;
    /// soil depth of all layers
    float **m_soilDepth;
	/// soil thickness of all layers
	float **m_soilThick;

    /// amount of water available to plants in soil layer at field capacity (fc - wp water), sol_fc in SWAT
    float **m_soilAWC;
    /// total m_soilAWC in soil profile, sol_sumfc in SWAT
    float *m_totSoilAWC;
    /// amount of water held in soil profile at saturation, sol_sumul in SWAT
    float *m_totSoilSat;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    float **m_soilStorage;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    float *m_soilStorageProfile;
    
    /**  crop or land cover related parameters  **/

	///amount of residue on soil surface (kg/ha)
	float *m_sol_rsdin;
	/// amount of residue on soil surface (10 mm surface)
	float *m_sol_cov;
	/// amount of organic matter in the soil layer classified as residue, sol_rsd |kg/ha in SWAT
	float **m_sol_rsd;
    /// land cover status code, 0 means no crop while 1 means land cover growing
    float *m_igro;
    /// land cover/crop  classification:1-7, i.e., IDC
    float *m_landCoverCls;
    /// minimum LAI during winter dormant period, alai_min
    float *m_aLAIMin;
    /// Radiation-use efficicency or biomass-energy ratio ((kg/ha)/(MJ/m**2)), BIO_E in SWAT
    float *m_BIOE;
    /// Biomass-energy ratio corresponding to the 2nd point on the radiation use efficiency curve
    float *m_BIOEHI;
    /// fraction of biomass that drops during dormancy (for tree only), bio_leaf
    float *m_frBioLeafDrop;
    /// maximum (potential) leaf area index (BLAI in cropLookup db)
    float *m_maxLAI;
    /// Maximum biomass for a forest (metric tons/ha), BMX_TREES in SWAT
    float *m_maxBiomass;
    /// nitrogen uptake parameter #1: normal fraction of N in crop biomass at emergence
    float *m_frPlantN1;
    /// nitrogen uptake parameter #2: normal fraction of N in crop biomass at 50% maturity
    float *m_frPlantN2;
    /// nitrogen uptake parameter #3: normal fraction of N in crop biomass at maturity
    float *m_frPlantN3;
    /// phosphorus uptake parameter #1: normal fraction of P in crop biomass at emergence
    float *m_frPlantP1;
    /// phosphorus uptake parameter #2: normal fraction of P in crop biomass at 50% maturity
    float *m_frPlantP2;
    /// phosphorus uptake parameter #3: normal fraction of P in crop biomass at maturity
    float *m_frPlantP3;
    /// maximum canopy height (m)
    float *m_chtMax;
    /// elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve
    float *m_co2Hi;
    /// fraction of growing season(PHU) when senescence becomes dominant
    float *m_frDLAI;
    /// plant water uptake compensation factor
    float *m_epco;
    /// light extinction coefficient, ext_coef
    float *m_lightExtCoef;
    /// fraction of the growing season corresponding to the 1st point on optimal leaf area development curve
    float *m_frGrowOptLAI1;
    /// fraction of the growing season corresponding to the 2nd point on optimal leaf area development curve
    float *m_frGrowOptLAI2;
    /// harvest index: crop yield/aboveground biomass (kg/ha)/(kg/ha)
    float *m_hvstIdx;
    /// fraction of maximum leaf area index corresponding to the 1st point on optimal leaf area development curve
    float *m_frMaxLAI1;
    /// fraction of maximum leaf area index corresponding to the 2nd point on optimal leaf area development curve
    float *m_frMaxLAI2;
    /// the number of years for the tree species to reach full development (years), MAT_YRS in SWAT
    float *m_matYrs;
    /// minimum temperature for plant growth
    float *m_tBase;
    /// optional temperature for plant growth
    float *m_tOpt;
    /// Rate of decline in radiation use efficiency per unit increase in vapor pressure deficit, wavp in SWAT
    float *m_wavp;

    /**  parameters need to be initialized in this module if they are NULL, i.e., in initialOutputs()  **/
    /// canopy height (m)
    float *m_cht;
    /// albedo in the current day
    float *m_albedo;
    /// initial age of trees (yrs) at the beginning of simulation
    float *m_initTreeMatYr;
    /// initial  dry weight biomass
    float *m_initBiomass;
    /// initial LAI
    float *m_initLAI;
    /// total heat units needed to bring plant to maturity
    float *m_PHUPlt;
    /// dominant code, 0 is land cover growing (not dormant), 1 is land cover dormant, by default m_dormFlag is 0.
    float *m_dormFlag;
    /// actual ET simulated during life of plant, plt_et in SWAT
    float *m_pltET;
    /// potential ET simulated during life of plant, pltPET in SWAT
    float *m_pltPET;

    /**  Nutrient related parameters  **/

    /// Nitrogen uptake distribution parameter
    float m_NUpDis;
    /// Phosphorus uptake distribution parameter
    float m_PUpDis;
    /// Nitrogen fixation coefficient, FIXCO in SWAT
    float m_NFixCoef;
    /// Maximum daily-n fixation (kg/ha), NFIXMX in SWAT
    float m_NFixMax;

    /**  input from other modules  **/

    /// day length
    float *m_dayLen;
    /// vapor pressure deficit (kPa)
    float *m_VPD;
    /// potential evapotranspiration, pet_day in SWAT
    float *m_PET;
    /// maximum plant et (mm H2O), ep_max in SWAT
    float *m_ppt;
    /// actual amount of evaporation (soil et), es_day in SWAT
    float *m_soilESDay;
    /// amount of nitrogen stored in the nitrate pool
    float **m_soilNO3;
    /// amount of phosphorus stored in solution
    float **m_soilPsol;
    /// amount of water in snow on current day
    float *m_snowAcc;
    /**  intermediate variables  **/

    /// water uptake distribution parameter, NOT ALLOWED TO MODIFIED BY USERS
    float ubw;
    /// water uptake normalization parameter, NOT ALLOWED TO MODIFIED BY USERS
    float uobw;
    /// current rooting depth
    float* m_soilRD;

    /**  set output variables  **/

    /// the leaf area indices for day i
    float *m_LAIDay;
    /// fraction of plant heat units (PHU) accumulated, also as output, phuacc in SWAT
    float *m_frPHUacc;
    /// maximum leaf area index for the current year (m_yearIdx), lai_yrmx in SWAT
    float *m_LAIYrMax;
    /// harvest index adjusted for water stress, hvstiadj in SWAT
    float *m_hvstIdxAdj;
    /// DO NOT KNOW NOW, initialized as 0.
    float *m_LAIMaxFr;
    /// DO NOT KNOW NOW
    float *m_oLAI;
    /// last soil root depth for use in harvest-kill-op/kill-op,
    float *m_lastSoilRootDepth;
    /// actual amount of transpiration (mm H2O), ep_day in SWAT
    float *m_plantEPDay;
    /// fraction of total plant biomass that is in roots, rwt in SWAT
    float *m_frRoot;
    /// amount of nitrogen added to soil via fixation on the day
    float *m_fixN;
    /// plant uptake of nitrogen, nplnt in SWAT
    float *m_plantUpTkN;
    /// plant uptake of phosphorus, pplnt in SWAT
    float *m_plantUpTkP;
    /// amount of nitrogen in plant biomass (kg/ha), plantn in SWAT
    float *m_plantN;
    /// amount of phosphorus in plant biomass (kg/ha), plantp in SWAT
    float *m_plantP;
    /// fraction of plant biomass that is nitrogen, pltfr_n in SWAT
    float *m_frPlantN;
    /// fraction of plant biomass that is phosphorus, pltfr_p in SWAT
    float *m_frPlantP;
    /// plant nitrogen deficiency (kg/ha), uno3d in SWAT
    float *m_NO3Defic;
    /// soil aeration stress
    float *m_frStrsAe;
    /// fraction of potential plant growth achieved where the reduction is caused by nitrogen stress, strsn in SWAT
    float *m_frStrsN;
    /// fraction of potential plant growth achieved where the reduction is caused by phosphorus stress, strsp in SWAT
    float *m_frStrsP;
    /// fraction of potential plant growth achieved where the reduction is caused by temperature stress, strstmp in SWAT
    float *m_frStrsTmp;
    /// fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT
    float *m_frStrsWa;
    /// biomass generated on current day, bioday in SWAT
    float *m_biomassDelta;
    /// land cover/crop biomass (dry weight), bio_ms in SWAT
    float *m_biomass;

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from swu.f of SWAT rev. 637
    //  Distribute potential plant evaporation through
    //	the root zone and calculates actual plant water use based on soil
    //	water availability. Also estimates water stress factor.
    //////////////////////////////////////////////////////////////////////////
    void DistributePlantET(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from grow.f of SWAT rev. 637
    //  Adjust plant biomass, leaf area index, and canopy height
    //	taking into account the effect of water, temperature and nutrient stresses  on the plant
    //////////////////////////////////////////////////////////////////////////
    void AdjustPlantGrowth(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from tstr.f of SWAT rev. 637
    //  Compute temperature stress for crop growth - strstmp
    //////////////////////////////////////////////////////////////////////////
    void CalTempStress(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from nup.f of SWAT rev. 637
    //  Calculates plant nitrogen uptake
    //////////////////////////////////////////////////////////////////////////
    void PlantNitrogenUptake(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from nfix.f of SWAT rev. 637
    //  Estimate nitrogen fixation by legumes
    //  wshd_fixn is NOT INCLUDED, average annual amount of nitrogen added to plant biomass via fixation
    //////////////////////////////////////////////////////////////////////////
    void PlantNitrogenFixed(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from npup.f of SWAT rev. 637
    //  Calculates plant phosphorus uptake
    //////////////////////////////////////////////////////////////////////////
    void PlantPhosphorusUptake(int i);

    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from dormant.f of SWAT rev. 637
    //  Check the dormant status of the different plant types
    //////////////////////////////////////////////////////////////////////////
    void CheckDormantStatus(int i);

    /// initialize output variables
    void initialOutputs();
};

/// the following two variables can be temporary variables to save memory.
///// 1st shape parameter for leaf area development equation
//float* m_LAIShpCoef1;
///// 2nd shape parameter for leaf area development equation
//float* m_LAIShpCoef2;
///// 1st shape parameter for radiation use efficiency equation, wac21 in SWAT
//float* m_RadUseEffiShpCoef1;
///// 2nd shape parameter for radiation use efficiency equation, wac22 in SWAT
//float* m_RadUseEffiShpCoef2;

/// currently, the following two variables are assigned the default value.
///// initial root to shoot ratio at beg of growing season, rsr1c in SWAT
//float* m_rootShootRatio1;
///// root to shoot ratio at end of growing season, rsr2c in SWAT
//float* m_rootShootRatio2;