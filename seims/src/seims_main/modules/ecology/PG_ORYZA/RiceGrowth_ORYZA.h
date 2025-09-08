/*!
 * \file RiceGrowth_ORYZA.h
 * \brief Rice crop growth module of ORYZA2000 model.
 *        Rewrite from APSIM-ORYZA(2), version august, 503, November 2002.
 *        The model is adapted from ORYZA1 (1995), and ORYZA_w(1996) models.
 *
 * Changelog:
 *   - 1. 2018-03-26 - sf - Initial implementation.
 *   - 2. 2018-06-12 - lj - Code review and reformat code style.
 *
 * \author Fang Shen
 */
#ifndef SEIMS_MODULE_PG_ORYZA_H
#define SEIMS_MODULE_PG_ORYZA_H

#include "SimulationModule.h"

/*! \defgroup ORYZA
 * \ingroup Ecology
 * \brief Rice crop growth module of ORYZA2000 model, version august, 503, Nov. 2002.
 */
class ORYZA: public SimulationModule {
public:
    ORYZA();

    ~ORYZA();

    int Execute() OVERRIDE;

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Get1DData(const char* key, int* n, int** data) OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, FLTPT** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void Get2DData(const char* key, int* n, int* col, FLTPT*** data) OVERRIDE;

private:
    /// valid cells number
    int m_nCells;

    /**  climate inputs  **/
    /// CO2 concentration
    FLTPT m_co2;
    /// mean air temperature
    FLTPT* m_meanTemp;
    /// max air temperature
    FLTPT* m_tMax;
    /// min air temperature
    FLTPT* m_tMin;
    /// solar radiation
    FLTPT* m_SR;

    /**  soil properties  **/
    /// soil layers,used to compute the water stress
    int* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// maximum root depth
    FLTPT* m_soilZMX;
    /// albedo when soil is moist
    FLTPT* m_soilALB;
    /// soil depth of all layers
    FLTPT** m_soilDepth;
    /// soil thickness of all layers
    FLTPT** m_soilThick;
    /// amount of water available to plants in soil layer at field capacity (fc - wp water), sol_fc in SWAT
    FLTPT** m_soilAWC;
    /// saturated soil water, mm
    FLTPT** m_sol_sat;
    ///water content of soil at -1.5 MPa (wilting point)
    FLTPT** m_soilWP;
    /// total m_soilAWC in soil profile, sol_sumfc in SWAT
    FLTPT* m_totSoilAWC;
    /// amount of water held in soil profile at saturation, sol_sumul in SWAT
    FLTPT* m_totSoilSat;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    FLTPT** m_soilStorage;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    FLTPT* m_soilWtrStoPrfl;
    /// last soil root depth for use in harvest-kill-op/kill-op,
    FLTPT* m_stoSoilRootD;
    /// amount of organic matter in the soil layer classified as residue, sol_rsd |kg/ha in SWAT
    FLTPT** m_soilRsd;
    /// amount of residue on soil surface (10 mm surface)
    FLTPT* m_rsdCovSoil;
    ///amount of residue on soil surface (kg/ha)
    FLTPT* m_sol_rsdin;
    /// albedo in the current day
    FLTPT* m_alb;
    /// amount of water in snow on current day
    FLTPT* m_snowAcc;
    /// fraction of potential plant growth achieved where the reduction is caused by water stress
    FLTPT* m_frStrsWtr;
    /// fraction of potential plant growth achieved where the reduction is caused by nitrogen stress
    FLTPT* m_frStrsN;

    /**  rice related parameters, read from the experimental file(rice_param_ini.txt)  **/
    /// the temperature params which control the growth of rice,base,optimum,max as follows
    FLTPT m_tbd;
    FLTPT m_tod;
    FLTPT m_tmd;
    /// the params of compute dvr
    FLTPT m_dvrj;
    FLTPT m_dvri;
    FLTPT m_dvrp;
    FLTPT m_dvrr;
    FLTPT m_mopp;
    FLTPT m_ppse;
    /// Delay parameter in phenology
    FLTPT m_shckd;
    /// extinction coefficient of N profile in the canopy as a function of development stage, but usually set as 0.4
    FLTPT m_knf;
    /// Maximum/Minimum relative growth rate of leaf area
    FLTPT m_rgrlMX;
    FLTPT m_rgrlMN;
    /// sow factors
    FLTPT m_nh;
    FLTPT m_nplh;
    FLTPT m_nplsb;
    /// Initial leaf area per plant
    FLTPT m_lape;
    /// Root depth at transplanting day (m)
    FLTPT m_zrttr;
    /// Temperature increase in seed-bed due to cover:Zero when no cover over seed-bed; 9.5 with seed-bed
    FLTPT m_tmpsb;
    /// the factors to compute the fraction of dry weight
    FLTPT m_aFsh;
    FLTPT m_bFsh;
    FLTPT m_aFlv;
    FLTPT m_bFlv;
    FLTPT m_aFso;
    FLTPT m_bFso;
    FLTPT m_aDrlv;
    FLTPT m_bDrlv;
    /// Time coefficient for loss of stem reserves (1 d-1)
    FLTPT m_tclstr;
    /// Factor accounting for increase in maintenance respiration with a 10 oC rise in temperature
    FLTPT m_q10;
    /// Reference temperature
    FLTPT m_tref;
    /// Maintenance respiration coefficient (kg CH2O kg-1 DM d-1)
    FLTPT m_mainLV;
    FLTPT m_mainST;
    FLTPT m_mainSO;
    FLTPT m_mainRT;
    /// Carbohydrate requirement for dry matter production (kg CH2O kg-1 DM leaf)
    FLTPT m_crgLV;
    FLTPT m_crgST;
    FLTPT m_crgSTR;
    FLTPT m_crgSO;
    FLTPT m_crgRT;
    /// Fraction of carbohydrates allocated to stems that is stored as reserves
    FLTPT m_fstr;
    /// Fraction of allocated stem reserves that is available for growth
    FLTPT m_lrstr;
    /// SLA function parameters:SLA = ASLA + BSLA*EXP(CSLA*(DVS-DSLA))
    FLTPT m_aSLA;
    FLTPT m_bSLA;
    FLTPT m_cSLA;
    FLTPT m_dSLA;
    /// maximum value of SLA (ha/kg)
    FLTPT m_slaMX;
    /// Carbon balance parameters, Mass fraction carbon (kg C kg-1 DM)
    FLTPT m_fcRT;
    FLTPT m_fcLV;
    FLTPT m_fcST;
    FLTPT m_fcSTR;
    FLTPT m_fcSO;
    /// Maximum individual grain weight (kg grain-1)
    FLTPT m_wgrMX;
    /// Growth rate of roots (m d-1)
    FLTPT m_gzrt;
    /// Maximum depth of roots if drought (m)
    FLTPT m_zrtMCD;
    /// Fraction of total shortwave irradiation that is photo-synthetically active (PAR)
    FLTPT m_frpar;
    /// Spikelet growth factor
    FLTPT m_spgf;
    /// function parameters of maximum leaf N fraction
    FLTPT m_nMaxL;
    /// function parameters of minimum leaf N fraction
    FLTPT m_nMinL;
    /// Residual N fraction of leaves (kg N kg-1 leaves)
    FLTPT m_rfnlv;
    /// Residual N fraction of stems (kg N kg-1 stems)
    FLTPT m_rfnst;
    /// Fraction N translocation from roots as (additonal) fraction of total N translocation from stems and leaves
    FLTPT m_fntrt;
    /// Time coefficient for N translocation to grains
    FLTPT m_tcntrf;
    /// Maximum N concentration in storage organs
    FLTPT m_nMaxSO;
    /// function parameters of minimum N concentration in storage organs
    FLTPT m_anMinSO;
    /// function parameters of minimum N concentration in storage organs
    FLTPT m_bnMinSO;
    /// Relation between seedling age and delay in leaf area development
    FLTPT m_shckl;
    /// Duration of seedbed
    int m_sbdur;
    /// Lower limit leaf rolling (kPa)
    FLTPT m_llls;
    /// Upper limit leaf rolling (kPa)
    FLTPT m_ulls;
    /// Lower limit leaf expansion (kPa)
    FLTPT m_llle;
    /// Upper limit leaf expansion (kPa)
    FLTPT m_ulle;
    /// Lower limit death of leaves (kPa)
    FLTPT m_lldl;
    /// Upper limit death of leaves (kPa)
    FLTPT m_uldl;

    /*   parameters used to compute ET and water stress   */

    /// maximum plant et (mm H2O)
    FLTPT* m_ppt;
    /// actual amount of transpiration (mm H2O)
    FLTPT* m_actPltET;
    /// plant water uptake compensation factor
    FLTPT* m_epco;
    /// Crop stage,0=before sowing; 1=sowing; 2=in seedbed; 3=day of transplanting; 4=main growth period, should be get value at PLTMGT_SWAT
    int* m_cropsta;
    /// Temperature sum
    FLTPT* m_ts;

    /*   parameters related to the current day and latitude, to describe the sun */

    ///latitude of the stations
    FLTPT* m_celllat;
    FLTPT m_cellLat;
    /// Photoperiodic daylength (base = -4 degrees)
    //FLTPT *m_dayLenP;
    /// Astronomical daylength (base = 0 degrees)
    FLTPT* m_dayL;
    /// Intermediate variable for subroutine Oryza_SSKYC
    FLTPT* m_sinLD;
    /// Intermediate variable for subroutine Oryza_SSKYC
    FLTPT* m_cosLD;
    /// Daily integral of sine of solar height corrected for lower transmission at low elevation
    FLTPT* m_dsinbe;
    /// Sine of solar height
    FLTPT* m_sinb;
    /// Solar constant at day=IDOY
    FLTPT* m_solcon;
    /// Instantaneous flux of diffuse photo-synthetically active irradiation (PAR)
    FLTPT* m_rdpdf;
    /// Instantaneous flux of direct photo-synthetically active radiation (PAR)
    FLTPT* m_rdpdr;

    /*  parameters related to the growth of rice   */

    /// Green area index above selected height
    FLTPT* m_gaid;
    /// Green area index
    FLTPT* m_gai;
    /// Absorbed flux for shaded leaves
    FLTPT* m_rapshl;
    /// Direct flux absorbed by leaves
    FLTPT* m_rapppl;
    /// Fraction of leaf area that is sunlit
    FLTPT* m_fslla;
    /// N fraction in leaves on leaf area basis (g N m-2 leaf)as a function of development stage
    FLTPT m_nflv;
    /// effect of temperature on AMAX (-; Y-value) as a function of temperature
    FLTPT m_redf;
    /// light use effiency (-; Y-value) as a function of temperature
    FLTPT m_eff;
    /// Instantaneous assimilation rate of leaves at depth GAI
    FLTPT* m_gpl;
    /// Absorbed radiation at depth GAI
    FLTPT* m_rapl;
    /// Instantaneous assimilation rate of whole canopy         kg CO2/ ha soil/h
    FLTPT* m_gpc;
    /// Absorbed PAR
    FLTPT* m_rapc;
    /// At the specified HOUR, external radiation conditions are computed
    FLTPT m_hour;
    /// Daily total gross assimilation
    FLTPT* m_gpcdt;
    /// Daily rate of absorbed PAR
    FLTPT* m_rapcdt;
    /// Daily total gross CO2 assimilation of crop  (kg CO2 ha-1 d-1)
    FLTPT* m_dtga;
    /// Rate of increase in spikelet number (no ha-1 d-1)
    FLTPT gnsp;
    /// Gross growth rate of the crop (kg DM/ha/d)
    FLTPT* m_gcr;
    /// the reduction factor: cold temp
    FLTPT* m_coldTT;
    /// the reduction factor: hot temp
    FLTPT* m_tfert;
    /// the sum day of hot
    FLTPT m_ntfert;
    /// Number of spikelets
    FLTPT* m_nsp;
    /// Rate of increase in grain number (no ha-1 d-1)
    FLTPT* m_gngr;
    /// Growth rate leaf area index
    FLTPT m_gLai;
    /// Green leaves growth rate (kg d-1 ha-1)
    FLTPT* m_rwlvg;
    /// Specific leaf area,computed by empirical factors
    FLTPT* m_sla;
    /// root length or root depth
    FLTPT* m_zrt;

    /// specific leaf area
    FLTPT m_sai;
    /// apparent leaf area index(including stem area)
    FLTPT m_aLAI;
    /// the fraction of dry matter to the shoot(FSH), leave(FLV), stems(FST), panicle(FSO), root(FRT)
    FLTPT m_fsh;
    FLTPT m_frt;
    FLTPT m_flv;
    FLTPT m_fst;
    FLTPT m_fso;
    FLTPT m_drlv;

    /// effect of N stress on leaf death rate
    FLTPT m_nsllv;
    /// the death or loss rate of stem
    FLTPT m_lstr;
    /// Factor accounting for effect of temperature on respiration
    FLTPT m_teff;
    /// Dry weight of dead leaves
    FLTPT* m_wlvd;
    /// dry weight of stems reserves
    FLTPT* m_wsts;
    /// dry weight of structural stems
    FLTPT* m_wstr;
    /// Number of grains
    FLTPT* m_ngr;
    /// Total net CO2 assimilation  kg CO2 ha-1
    FLTPT* m_tnass;
    /// Dry weight of leaves
    FLTPT* m_wlv;
    /// Total aboveground dry matter
    FLTPT* m_wagt;
    /// land cover/crop biomass (dry weight)
    FLTPT* m_biomass;
    /// fraction of total plant biomass that is in roots
    FLTPT* m_frRoot;
    /// growth rate of leaf
    FLTPT glv;
    /// growth rate of stem
    FLTPT gst;
    // growth rate of storage organs
    FLTPT gso;
    /// dry weight of stems
    FLTPT* m_wst;
    /// dry weight of storage organs
    FLTPT* m_wso;
    /// Dry weight of green leaves  kg / ha
    FLTPT* m_wlvg;
    /// Dry weight of roots
    FLTPT* m_wrt;

    /*  parameter related to the N of plant and soil   */
    /// amount of nitrogen stored in the nitrate pool
    FLTPT** m_soilNO3;
    /// amount of nitrogen in plant biomass (kg/ha), from Biomass_EPIC
    FLTPT* m_pltN;
    /// amount of nitrogen in stem
    FLTPT* m_anst;
    /// amount of N uptake by plant
    FLTPT* m_plantUpTkN;
    /// Amount of N in crop till flowering
    FLTPT* m_ancrf;
    /// Amount of N in leaves
    FLTPT* m_anlv;
    /// the day of sowing
    int sowDay;

    /**  rice related parameters, output  **/
    /// Development stage of the crop
    FLTPT* m_dvs;
    /// Leaf area index
    FLTPT* m_lai;
    /// Dry weight of rough rice (final yield)
    FLTPT* m_wrr;


    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SUBDD.f
    //  calculates the daily amount of heat units  for calculation of the phenological development rate and early leaf area growth
    //////////////////////////////////////////////////////////////////////////
    FLTPT CalHeatUnitDaily(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from PHENOL.f
    //  calculates the rate of phenological development of the crop based on photoperiod and temperature
    //////////////////////////////////////////////////////////////////////////
    FLTPT CalDevelopmentRate(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SASTRO.f
    //  calculates solar constant, daily  extraterrestrial radiation, daylength and some intermediate variables required by other routines
    //////////////////////////////////////////////////////////////////////////
    void CalDayLengthAndSINB(int i);
    //////////////////////////////////////////////////////////////////////////
    //  estimates solar inclination and fluxes of  diffuse and direct irradiation at a particular time of the day
    //////////////////////////////////////////////////////////////////////////
    void CalDirectRadiation(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SRDPRF.f
    //  calculates the absorbed flux of radiation for shaded leaves, the direct flux absorbed by leaves and the fraction of sunlit leaf area
    //////////////////////////////////////////////////////////////////////////
    void CalLeafAbsorbRadiation(int i);

    FLTPT CalLeafMaxAssimilationRate(FLTPT gai, FLTPT gaid, FLTPT nflv, FLTPT redf);
    //////////////////////////////////////////////////////////////////////////
    //  calculates assimilation at a single depth in the canopy
    //////////////////////////////////////////////////////////////////////////
    void Sgpl(int i);
    //////////////////////////////////////////////////////////////////////////
    //  performs a Gaussian integration over depth of canopy by selecting three different GAI's and computing assimilation at these GAI levels
    //////////////////////////////////////////////////////////////////////////
    void CalCanopyAssimilationRate(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SGPCDT.f
    //  calculates daily total gross assimilation (GPCDT) by performing a Gaussian integration over time. At three different times of the day,
    //  radiation is computed and used to determine assimilation whereafter integration takes place. three point GAUSS procedure is used
    //////////////////////////////////////////////////////////////////////////
    void CalDailyCanopyPhotosynthesisRate(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SUBGRN.f
    //  calculates spikelet formation rate and spikelet fertility as affected by low and high temperature(a function of temperature when dvs = 1) and the grain growth rate
    //////////////////////////////////////////////////////////////////////////
    void CalSpikeletAndGrainRate(int i);
    //////////////////////////////////////////////////////////////////////////
    //  calculates the rate of growth of LAI of of the crop in the seedbed and after transplanting in the field
    //  Reductions by N-stress and water-stress are taken into account
    //////////////////////////////////////////////////////////////////////////
    void LAI(int i);
    //////////////////////////////////////////////////////////////////////////
    //  the main part to compute LAI, wso(yield)
    //////////////////////////////////////////////////////////////////////////
    void CalRiceGrowth(int i);

    //////////////////////////////////////////////////////////////////////////
    //  Distribute potential plant evaporation through
    //	the root zone and calculates actual plant water use based on soil
    //	water availability. Also estimates water stress factor.
    //////////////////////////////////////////////////////////////////////////
    void CalPlantETAndWStress(int i);
    //////////////////////////////////////////////////////////////////////////
    //  Calculates plant nitrogen uptake
    //////////////////////////////////////////////////////////////////////////
    void CalPlantNUptake(int i);
};
#endif /* SEIMS_MODULE_PG_ORYZA_H */
