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

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void Get2DData(const char* key, int* n, int* col, float*** data) OVERRIDE;

private:
    /// valid cells number
    int m_nCells;

    /**  climate inputs  **/
    /// CO2 concentration
    float m_co2;
    /// mean air temperature
    float* m_meanTemp;
    /// max air temperature
    float* m_tMax;
    /// min air temperature
    float* m_tMin;
    /// solar radiation
    float* m_SR;

    /**  soil properties  **/
    /// soil layers,used to compute the water stress
    float* m_nSoilLyrs;
    /// maximum soil layers
    int m_maxSoilLyrs;
    /// maximum root depth
    float* m_soilZMX;
    /// albedo when soil is moist
    float* m_soilALB;
    /// soil depth of all layers
    float** m_soilDepth;
    /// soil thickness of all layers
    float** m_soilThick;
    /// amount of water available to plants in soil layer at field capacity (fc - wp water), sol_fc in SWAT
    float** m_soilAWC;
    /// saturated soil water, mm
    float** m_sol_sat;
    ///water content of soil at -1.5 MPa (wilting point)
    float** m_soilWP;
    /// total m_soilAWC in soil profile, sol_sumfc in SWAT
    float* m_totSoilAWC;
    /// amount of water held in soil profile at saturation, sol_sumul in SWAT
    float* m_totSoilSat;
    /// amount of water stored in soil layers on current day, sol_st in SWAT
    float** m_soilStorage;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    float* m_soilWtrStoPrfl;
    /// last soil root depth for use in harvest-kill-op/kill-op,
    float* m_stoSoilRootD;
    /// amount of organic matter in the soil layer classified as residue, sol_rsd |kg/ha in SWAT
    float** m_soilRsd;
    /// amount of residue on soil surface (10 mm surface)
    float* m_rsdCovSoil;
    ///amount of residue on soil surface (kg/ha)
    float* m_sol_rsdin;
    /// albedo in the current day
    float* m_alb;
    /// amount of water in snow on current day
    float* m_snowAcc;
    /// fraction of potential plant growth achieved where the reduction is caused by water stress
    float* m_frStrsWtr;
    /// fraction of potential plant growth achieved where the reduction is caused by nitrogen stress
    float* m_frStrsN;

    /**  rice related parameters, read from the experimental file(rice_param_ini.txt)  **/
    /// the temperature params which control the growth of rice,base,optimum,max as follows
    float m_tbd;
    float m_tod;
    float m_tmd;
    /// the params of compute dvr
    float m_dvrj;
    float m_dvri;
    float m_dvrp;
    float m_dvrr;
    float m_mopp;
    float m_ppse;
    /// Delay parameter in phenology
    float m_shckd;
    /// extinction coefficient of N profile in the canopy as a function of development stage, but usually set as 0.4
    float m_knf;
    /// Maximum/Minimum relative growth rate of leaf area
    float m_rgrlMX;
    float m_rgrlMN;
    /// sow factors
    float m_nh;
    float m_nplh;
    float m_nplsb;
    /// Initial leaf area per plant
    float m_lape;
    /// Root depth at transplanting day (m)
    float m_zrttr;
    /// Temperature increase in seed-bed due to cover:Zero when no cover over seed-bed; 9.5 with seed-bed
    float m_tmpsb;
    /// the factors to compute the fraction of dry weight
    float m_aFsh;
    float m_bFsh;
    float m_aFlv;
    float m_bFlv;
    float m_aFso;
    float m_bFso;
    float m_aDrlv;
    float m_bDrlv;
    /// Time coefficient for loss of stem reserves (1 d-1)
    float m_tclstr;
    /// Factor accounting for increase in maintenance respiration with a 10 oC rise in temperature
    float m_q10;
    /// Reference temperature
    float m_tref;
    /// Maintenance respiration coefficient (kg CH2O kg-1 DM d-1)
    float m_mainLV;
    float m_mainST;
    float m_mainSO;
    float m_mainRT;
    /// Carbohydrate requirement for dry matter production (kg CH2O kg-1 DM leaf)
    float m_crgLV;
    float m_crgST;
    float m_crgSTR;
    float m_crgSO;
    float m_crgRT;
    /// Fraction of carbohydrates allocated to stems that is stored as reserves
    float m_fstr;
    /// Fraction of allocated stem reserves that is available for growth
    float m_lrstr;
    /// SLA function parameters:SLA = ASLA + BSLA*EXP(CSLA*(DVS-DSLA))
    float m_aSLA;
    float m_bSLA;
    float m_cSLA;
    float m_dSLA;
    /// maximum value of SLA (ha/kg)
    float m_slaMX;
    /// Carbon balance parameters, Mass fraction carbon (kg C kg-1 DM)
    float m_fcRT;
    float m_fcLV;
    float m_fcST;
    float m_fcSTR;
    float m_fcSO;
    /// Maximum individual grain weight (kg grain-1)
    float m_wgrMX;
    /// Growth rate of roots (m d-1)
    float m_gzrt;
    /// Maximum depth of roots if drought (m)
    float m_zrtMCD;
    /// Fraction of total shortwave irradiation that is photo-synthetically active (PAR)
    float m_frpar;
    /// Spikelet growth factor
    float m_spgf;
    /// function parameters of maximum leaf N fraction
    float m_nMaxL;
    /// function parameters of minimum leaf N fraction
    float m_nMinL;
    /// Residual N fraction of leaves (kg N kg-1 leaves)
    float m_rfnlv;
    /// Residual N fraction of stems (kg N kg-1 stems)
    float m_rfnst;
    /// Fraction N translocation from roots as (additonal) fraction of total N translocation from stems and leaves
    float m_fntrt;
    /// Time coefficient for N translocation to grains
    float m_tcntrf;
    /// Maximum N concentration in storage organs
    float m_nMaxSO;
    /// function parameters of minimum N concentration in storage organs
    float m_anMinSO;
    /// function parameters of minimum N concentration in storage organs
    float m_bnMinSO;
    /// Relation between seedling age and delay in leaf area development
    float m_shckl;
    /// Duration of seedbed
    int m_sbdur;
    /// Lower limit leaf rolling (kPa)
    float m_llls;
    /// Upper limit leaf rolling (kPa)
    float m_ulls;
    /// Lower limit leaf expansion (kPa)
    float m_llle;
    /// Upper limit leaf expansion (kPa)
    float m_ulle;
    /// Lower limit death of leaves (kPa)
    float m_lldl;
    /// Upper limit death of leaves (kPa)
    float m_uldl;

    /*   parameters used to compute ET and water stress   */

    /// maximum plant et (mm H2O)
    float* m_ppt;
    /// actual amount of transpiration (mm H2O)
    float* m_actPltET;
    /// plant water uptake compensation factor
    float* m_epco;
    /// Crop stage,0=before sowing; 1=sowing; 2=in seedbed; 3=day of transplanting; 4=main growth period, should be get value at PLTMGT_SWAT
    float* m_cropsta;
    /// Temperature sum
    float* m_ts;

    /*   parameters related to the current day and latitude, to describe the sun */

    ///latitude of the stations
    float* m_celllat;
    float m_cellLat;
    /// Photoperiodic daylength (base = -4 degrees)
    //float *m_dayLenP;
    /// Astronomical daylength (base = 0 degrees)
    float* m_dayL;
    /// Intermediate variable for subroutine Oryza_SSKYC
    float* m_sinLD;
    /// Intermediate variable for subroutine Oryza_SSKYC
    float* m_cosLD;
    /// Daily integral of sine of solar height corrected for lower transmission at low elevation
    float* m_dsinbe;
    /// Sine of solar height
    float* m_sinb;
    /// Solar constant at day=IDOY
    float* m_solcon;
    /// Instantaneous flux of diffuse photo-synthetically active irradiation (PAR)
    float* m_rdpdf;
    /// Instantaneous flux of direct photo-synthetically active radiation (PAR)
    float* m_rdpdr;

    /*  parameters related to the growth of rice   */

    /// Green area index above selected height
    float* m_gaid;
    /// Green area index
    float* m_gai;
    /// Absorbed flux for shaded leaves
    float* m_rapshl;
    /// Direct flux absorbed by leaves
    float* m_rapppl;
    /// Fraction of leaf area that is sunlit
    float* m_fslla;
    /// N fraction in leaves on leaf area basis (g N m-2 leaf)as a function of development stage
    float m_nflv;
    /// effect of temperature on AMAX (-; Y-value) as a function of temperature
    float m_redf;
    /// light use effiency (-; Y-value) as a function of temperature
    float m_eff;
    /// Instantaneous assimilation rate of leaves at depth GAI
    float* m_gpl;
    /// Absorbed radiation at depth GAI
    float* m_rapl;
    /// Instantaneous assimilation rate of whole canopy         kg CO2/ ha soil/h
    float* m_gpc;
    /// Absorbed PAR
    float* m_rapc;
    /// At the specified HOUR, external radiation conditions are computed
    float m_hour;
    /// Daily total gross assimilation
    float* m_gpcdt;
    /// Daily rate of absorbed PAR
    float* m_rapcdt;
    /// Daily total gross CO2 assimilation of crop  (kg CO2 ha-1 d-1)
    float* m_dtga;
    /// Rate of increase in spikelet number (no ha-1 d-1)
    float gnsp;
    /// Gross growth rate of the crop (kg DM/ha/d)
    float* m_gcr;
    /// the reduction factor: cold temp
    float* m_coldTT;
    /// the reduction factor: hot temp
    float* m_tfert;
    /// the sum day of hot
    float m_ntfert;
    /// Number of spikelets
    float* m_nsp;
    /// Rate of increase in grain number (no ha-1 d-1)
    float* m_gngr;
    /// Growth rate leaf area index
    float m_gLai;
    /// Green leaves growth rate (kg d-1 ha-1)
    float* m_rwlvg;
    /// Specific leaf area,computed by empirical factors
    float* m_sla;
    /// root length or root depth
    float* m_zrt;

    /// specific leaf area
    float m_sai;
    /// apparent leaf area index(including stem area)
    float m_aLAI;
    /// the fraction of dry matter to the shoot(FSH), leave(FLV), stems(FST), panicle(FSO), root(FRT)
    float m_fsh;
    float m_frt;
    float m_flv;
    float m_fst;
    float m_fso;
    float m_drlv;

    /// effect of N stress on leaf death rate
    float m_nsllv;
    /// the death or loss rate of stem
    float m_lstr;
    /// Factor accounting for effect of temperature on respiration
    float m_teff;
    /// Dry weight of dead leaves
    float* m_wlvd;
    /// dry weight of stems reserves
    float* m_wsts;
    /// dry weight of structural stems
    float* m_wstr;
    /// Number of grains
    float* m_ngr;
    /// Total net CO2 assimilation  kg CO2 ha-1
    float* m_tnass;
    /// Dry weight of leaves
    float* m_wlv;
    /// Total aboveground dry matter
    float* m_wagt;
    /// land cover/crop biomass (dry weight)
    float* m_biomass;
    /// fraction of total plant biomass that is in roots
    float* m_frRoot;
    /// growth rate of leaf
    float glv;
    /// growth rate of stem
    float gst;
    // growth rate of storage organs
    float gso;
    /// dry weight of stems
    float* m_wst;
    /// dry weight of storage organs
    float* m_wso;
    /// Dry weight of green leaves  kg / ha
    float* m_wlvg;
    /// Dry weight of roots
    float* m_wrt;

    /*  parameter related to the N of plant and soil   */
    /// amount of nitrogen stored in the nitrate pool
    float** m_soilNO3;
    /// amount of nitrogen in plant biomass (kg/ha), from Biomass_EPIC
    float* m_pltN;
    /// amount of nitrogen in stem
    float* m_anst;
    /// amount of N uptake by plant
    float* m_plantUpTkN;
    /// Amount of N in crop till flowering
    float* m_ancrf;
    /// Amount of N in leaves
    float* m_anlv;
    /// the day of sowing
    int sowDay;

    /**  rice related parameters, output  **/
    /// Development stage of the crop
    float* m_dvs;
    /// Leaf area index
    float* m_lai;
    /// Dry weight of rough rice (final yield)
    float* m_wrr;


    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from SUBDD.f
    //  calculates the daily amount of heat units  for calculation of the phenological development rate and early leaf area growth
    //////////////////////////////////////////////////////////////////////////
    float CalHeatUnitDaily(int i);
    //////////////////////////////////////////////////////////////////////////
    //  The following code is transferred from PHENOL.f
    //  calculates the rate of phenological development of the crop based on photoperiod and temperature
    //////////////////////////////////////////////////////////////////////////
    float CalDevelopmentRate(int i);
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

    float CalLeafMaxAssimilationRate(float gai, float gaid, float nflv, float redf);
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
