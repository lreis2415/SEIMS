/*!
 * \file Biomass_EPIC.h
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637, plantmod.f
 *
 * Changelog:
 *   - 1. 2016-06-15 - lj - Initial implementation.
 *   - 2. 2016-10-07 - lj - Add some code of CENTURY model calculation.
 *   - 3. 2018-05-14 - lj - Code review and reformat code style.
 *   - 4. 2022-08-22 - lj - Change float to FLTPT.
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_MODULE_PG_EPIC_H
#define SEIMS_MODULE_PG_EPIC_H

#include "SimulationModule.h"

/** \defgroup PG_EPIC
 * \ingroup Ecology
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637
 */
/*!
 * \class Biomass_EPIC
 * \ingroup PG_EPIC
 * \brief Predicts daily potential growth of total plant biomass and roots and calculates leaf area index
 * incorporated a simplified version of the EPIC plant growth model as in SWAT rev. 637
 */
class Biomass_EPIC: public SimulationModule {
public:
    Biomass_EPIC();

    ~Biomass_EPIC();

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    void Get1DData(const char* key, int* n, int** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, FLTPT*** data) OVERRIDE;

private:
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

private:
    /// valid cells number
    int m_nCells;

    /**  climate inputs  **/

    /// CO2 concentration
    FLTPT m_co2Conc;
    /// mean air temperature
    FLTPT* m_meanTemp;
    /// min air temperature
    FLTPT* m_minTemp;
    /// solar radiation
    FLTPT* m_SR;
    /// average annual air temperature
    FLTPT* m_annMeanTemp;
    /// minimum day length
    FLTPT* m_dayLenMin;
    /// dormancy threshold
    FLTPT* m_dormHr;
    /**  soil properties  **/

    /// soil layers
    int* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// maximum root depth
    FLTPT* m_soilMaxRootD;
    /// albedo when soil is moist
    FLTPT* m_soilAlb;
    /// soil depth of all layers
    FLTPT** m_soilDepth;
    /// soil thickness of all layers
    FLTPT** m_soilThk;

    /// amount of water available to plants in soil layer at field capacity (fc - wp water), sol_fc in SWAT
    FLTPT** m_soilFC;
    /// total m_soilAWC in soil profile, sol_sumfc in SWAT
    FLTPT* m_soilSumFC;
    /// amount of water held in soil profile at saturation, sol_sumul in SWAT
    FLTPT* m_soilSumSat;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    FLTPT** m_soilWtrSto;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    FLTPT* m_soilWtrStoPrfl;

    /**  crop or land cover related parameters  **/

    ///amount of residue on soil surface (kg/ha)
    FLTPT* m_rsdInitSoil;
    /// amount of residue on soil surface (10 mm surface)
    FLTPT* m_rsdCovSoil;
    /// amount of organic matter in the soil layer classified as residue, sol_rsd |kg/ha in SWAT
    FLTPT** m_soilRsd;
    /// biomass target, kg/ha
    FLTPT* m_biomTrgt;
    /// land cover status code, 0 means no crop while 1 means land cover growing
    int* m_igro;
    /// land cover/crop  classification:1-7, i.e., IDC
    int* m_landCoverCls;
    /// minimum LAI during winter dormant period, alai_min
    FLTPT* m_minLaiDorm;
    /// Radiation-use efficicency or biomass-energy ratio ((kg/ha)/(MJ/m**2)), BIO_E in SWAT
    FLTPT* m_biomEnrgRatio;
    /// Biomass-energy ratio corresponding to the 2nd point on the radiation use efficiency curve
    FLTPT* m_biomEnrgRatio2ndPt;
    /// fraction of biomass that drops during dormancy (for tree only), bio_leaf
    FLTPT* m_biomDropFr;
    /// maximum (potential) leaf area index (BLAI in cropLookup db)
    FLTPT* m_maxLai;
    /// Maximum biomass for a forest (metric tons/ha), BMX_TREES in SWAT
    FLTPT* m_maxBiomTree;
    /// nitrogen uptake parameter #1: normal fraction of N in crop biomass at emergence
    FLTPT* m_biomNFr1;
    /// nitrogen uptake parameter #2: normal fraction of N in crop biomass at 50% maturity
    FLTPT* m_biomNFr2;
    /// nitrogen uptake parameter #3: normal fraction of N in crop biomass at maturity
    FLTPT* m_biomNFr3;
    /// phosphorus uptake parameter #1: normal fraction of P in crop biomass at emergence
    FLTPT* m_biomPFr1;
    /// phosphorus uptake parameter #2: normal fraction of P in crop biomass at 50% maturity
    FLTPT* m_biomPFr2;
    /// phosphorus uptake parameter #3: normal fraction of P in crop biomass at maturity
    FLTPT* m_biomPFr3;
    /// maximum canopy height (m)
    FLTPT* m_maxCanHgt;
    /// elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve
    FLTPT* m_co2Conc2ndPt;
    /// fraction of growing season(PHU) when senescence becomes dominant
    FLTPT* m_dormPHUFr;
    /// plant water uptake compensation factor
    FLTPT* m_epco;
    /// light extinction coefficient, ext_coef
    FLTPT* m_lightExtCoef;
    /// fraction of the growing season corresponding to the 1st point on optimal leaf area development curve
    FLTPT* m_frGrow1stPt;
    /// fraction of the growing season corresponding to the 2nd point on optimal leaf area development curve
    FLTPT* m_frGrow2ndPt;
    /// harvest index: crop yield/aboveground biomass (kg/ha)/(kg/ha)
    FLTPT* m_hvstIdx;
    /// fraction of maximum leaf area index corresponding to the 1st point on optimal leaf area development curve
    FLTPT* m_frMaxLai1stPt;
    /// fraction of maximum leaf area index corresponding to the 2nd point on optimal leaf area development curve
    FLTPT* m_frMaxLai2ndPt;
    /// the number of years for the tree species to reach full development (years), MAT_YRS in SWAT
    FLTPT* m_matYrs;
    /// minimum temperature for plant growth
    FLTPT* m_pgTempBase;
    /// optional temperature for plant growth
    FLTPT* m_pgOptTemp;
    /// Rate of decline in radiation use efficiency per unit increase in vapor pressure deficit, wavp in SWAT
    FLTPT* m_wavp;

    /**  parameters need to be initialized in this module if they are NULL, i.e., in initialOutputs(void)  **/
    /// canopy height (m)
    FLTPT* m_canHgt;
    /// albedo in the current day
    FLTPT* m_alb;
    /// initial age of trees (yrs) at the beginning of simulation
    FLTPT* m_curYrMat;
    /// initial biomass of transplants, kg/ha
    FLTPT* m_initBiom;
    /// initial leaf area index of transplants
    FLTPT* m_initLai;
    /// total heat units needed to bring plant to maturity
    FLTPT* m_phuPlt;
    /// dominant code, 0 is land cover growing (not dormant), 1 is land cover dormant, by default m_dormFlag is 0.
    int* m_dormFlag;
    /// actual ET simulated during life of plant, plt_et in SWAT
    FLTPT* m_totActPltET;
    /// potential ET simulated during life of plant, pltPET in SWAT
    FLTPT* m_totPltPET;

    /**  Nutrient related parameters  **/

    /// Nitrogen uptake distribution parameter
    FLTPT m_upTkDistN;
    /// Phosphorus uptake distribution parameter
    FLTPT m_upTkDistP;
    /// Nitrogen fixation coefficient, FIXCO in SWAT
    FLTPT m_NFixCoef;
    /// Maximum daily-n fixation (kg/ha), NFIXMX in SWAT
    FLTPT m_NFixMax;

    /**  input from other modules  **/

    /// day length
    FLTPT* m_dayLen;
    /// vapor pressure deficit (kPa)
    FLTPT* m_vpd;
    /// potential evapotranspiration, pet_day in SWAT
    FLTPT* m_pet;
    /// maximum plant et (mm H2O), ep_max in SWAT
    FLTPT* m_maxPltET;
    /// actual amount of evaporation (soil et), es_day in SWAT
    FLTPT* m_soilET;
    /// amount of nitrogen stored in the nitrate pool
    FLTPT** m_soilNO3;
    /// amount of phosphorus stored in solution
    FLTPT** m_soilSolP;
    /// amount of water in snow on current day
    FLTPT* m_snowAccum;
    /**  intermediate variables  **/

    /// water uptake distribution parameter, NOT ALLOWED TO MODIFIED BY USERS
    FLTPT ubw;
    /// water uptake normalization parameter, NOT ALLOWED TO MODIFIED BY USERS
    FLTPT uobw;
    /// current rooting depth
    FLTPT* m_pltRootD;
    /// wuse in DistributePlantET().
    FLTPT** m_wuse;

    /**  set output variables  **/

    /// the leaf area indices for day i
    FLTPT* m_lai;
    /// fraction of plant heat units (PHU) accumulated, also as output, phuacc in SWAT
    FLTPT* m_phuAccum;
    /// maximum leaf area index for the current year (m_yearIdx), lai_yrmx in SWAT
    FLTPT* m_maxLaiYr;
    /// harvest index adjusted for water stress, hvstiadj in SWAT
    FLTPT* m_hvstIdxAdj;
    /// DO NOT KNOW NOW, initialized as 0.
    FLTPT* m_LaiMaxFr;
    /// DO NOT KNOW NOW
    FLTPT* m_oLai;
    /// last soil root depth for use in harvest-kill-op/kill-op,
    FLTPT* m_stoSoilRootD;
    /// actual amount of transpiration (mm H2O), ep_day in SWAT
    FLTPT* m_actPltET;
    /// fraction of total plant biomass that is in roots, rwt in SWAT
    FLTPT* m_frRoot;
    /// amount of nitrogen added to soil via fixation on the day
    FLTPT* m_fixN;
    /// plant uptake of nitrogen, nplnt in SWAT
    FLTPT* m_plantUpTkN;
    /// plant uptake of phosphorus, pplnt in SWAT
    FLTPT* m_plantUpTkP;
    /// amount of nitrogen in plant biomass (kg/ha), plantn in SWAT
    FLTPT* m_pltN;
    /// amount of phosphorus in plant biomass (kg/ha), plantp in SWAT
    FLTPT* m_pltP;
    /// fraction of plant biomass that is nitrogen, pltfr_n in SWAT
    FLTPT* m_frPltN;
    /// fraction of plant biomass that is phosphorus, pltfr_p in SWAT
    FLTPT* m_frPltP;
    /// plant nitrogen deficiency (kg/ha), uno3d in SWAT
    FLTPT* m_NO3Defic;
    /// soil aeration stress
    FLTPT* m_frStrsAe;
    /// fraction of potential plant growth achieved where the reduction is caused by nitrogen stress, strsn in SWAT
    FLTPT* m_frStrsN;
    /// fraction of potential plant growth achieved where the reduction is caused by phosphorus stress, strsp in SWAT
    FLTPT* m_frStrsP;
    /// fraction of potential plant growth achieved where the reduction is caused by temperature stress, strstmp in SWAT
    FLTPT* m_frStrsTmp;
    /// fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT
    FLTPT* m_frStrsWtr;
    /// biomass generated on current day, bioday in SWAT
    FLTPT* m_biomassDelta;
    /// land cover/crop biomass (dry weight), bio_ms in SWAT
    FLTPT* m_biomass;
};
#endif /* SEIMS_MODULE_PG_EPIC_H */
