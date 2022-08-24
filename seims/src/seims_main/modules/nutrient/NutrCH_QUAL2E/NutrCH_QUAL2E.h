/*!
 * \file NutrCH_QUAL2E.h
 * \brief Calculates in-stream nutrient transformations with QUAL2E method.
 *        watqual2.f of SWAT
 *
 * Changelog:
 *   - 1. 2016-06-30 - hr - Initial implementation.
 *   - 2. 2017-12-26 - lj -
 *        -# Add point source loadings nutrients from Scenario.
 *        -# Add ammonian transported by surface runoff.
 *        -# Reformat code style. Update clsReaches usage.
 *   - 3. 2018-03-23 - lj - Debug for mpi version.
 *   - 4. 2018-05-15 - lj -
 *        -# Remove LayeringMethod variable and m_qUpReach, which are useless.
 *        -# Code review and reformat.
 *   - 5. 2022-08-22 - lj - Change float to FLTPT.
 *
 * \author Huiran Gao, Junzhi Liu, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_NUTRCH_QUAL2E_H
#define SEIMS_MODULE_NUTRCH_QUAL2E_H

#include "SimulationModule.h"

/** \defgroup NutrCH_QUAL2E
 * \ingroup Nutrient
 * \brief Calculates in-stream nutrient transformations with QUAL2E method.
 */

/*!
 * \class NutrCH_QUAL2E
 * \ingroup NutrCH_QUAL2E
 *
 * \brief Calculates the concentration of nutrient in reach using QUAL2E method.
 *
 */
class NutrCH_QUAL2E: public SimulationModule {
public:
    NutrCH_QUAL2E();

    ~NutrCH_QUAL2E();

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, FLTPT data) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, FLTPT* value) OVERRIDE;

    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

private:
    bool CheckInputCellSize(const char* key, int n);

    void AddInputNutrient(int i);

    void RouteOut(int i);

    void NutrientTransform(int i);

    /*!
    * \brief Corrects rate constants for temperature.
    *
    *    r20         1/day         value of the reaction rate coefficient at the standard temperature (20 degrees C)
    *    thk         none          temperature adjustment factor (empirical constant for each reaction coefficient)
    *    tmp         deg C         temperature on current day
    *
    * \return FLTPT
    */
    FLTPT corTempc(FLTPT r20, FLTPT thk, FLTPT tmp);

    /// Calculate average day length, solar radiation, and temperature for each channel
    void ParametersSubbasinForChannel();

    void PointSourceLoading();
private:
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    // cell number
    int m_nCells;
    /// time step (sec)
    int m_dt;
    /// downstream id (The value is 0 if there if no downstream reach)
    int* m_reachDownStream;
    /// Index of upstream Ids (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;
    /// reaches number
    int m_nReaches;
    /* reach up-down layering
     * key: stream order
     * value: reach ID of current stream order
     */
    map<int, vector<int> > m_reachLayers;
    /// scenario data

    /* point source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: point source management factory instance
     */
    map<int, BMPPointSrcFactory *> m_ptSrcFactory;

    /// input data

    FLTPT m_ai0; ///< ratio of chlorophyll-a to algal biomass (ug chla/mg alg)
    FLTPT m_ai1; ///< fraction of algal biomass that is nitrogen (mg N/mg alg)
    FLTPT m_ai2; ///< fraction of algal biomass that is phosphorus (mg P/mg alg)
    FLTPT m_ai3; ///< the rate of oxygen production per unit of algal photosynthesis (mg O2/mg alg)
    FLTPT m_ai4; ///< the rate of oxygen uptake per unit of algae respiration (mg O2/mg alg)
    FLTPT m_ai5; ///< the rate of oxygen uptake per unit of NH3 nitrogen oxidation (mg O2/mg N)
    FLTPT m_ai6; ///< the rate of oxygen uptake per unit of NO2 nitrogen oxidation (mg O2/mg N)

    FLTPT m_lambda0; ///< non-algal portion of the light extinction coefficient
    FLTPT m_lambda1; ///< linear algal self-shading coefficient
    FLTPT m_lambda2; ///< nonlinear algal self-shading coefficient

    FLTPT m_k_l;   ///< half saturation coefficient for light (MJ/(m2*hr))
    FLTPT m_k_n;   ///< half-saturation constant for nitrogen (mg N/L)
    FLTPT m_k_p;   ///< half saturation constant for phosphorus (mg P/L)
    FLTPT m_p_n;   ///< algal preference factor for ammonia
    FLTPT tfact;   ///< fraction of solar radiation computed in the temperature heat balance
                   ///< that is photo synthetically active
    FLTPT m_rnum1; ///< fraction of overland flow
    /// option for calculating the local specific growth rate of algae
    //     1: multiplicative:     u = mumax * fll * fnn * fpp
    //     2: limiting nutrient: u = mumax * fll * Min(fnn, fpp)
    //     3: harmonic mean: u = mumax * fll * 2. / ((1/fnn)+(1/fpp))
    int igropt;
    /// maximum specific algal growth rate at 20 deg C
    FLTPT m_mumax;
    /// algal respiration rate at 20 deg C (1/day)
    FLTPT m_rhoq;
    /// Conversion factor
    FLTPT m_cod_n;
    /// Reaction coefficient
    FLTPT m_cod_k;

    /// stream link
    int* m_rchID;
    /// soil temperature (deg C)
    FLTPT* m_soilTemp;
    /// day length for current day (h)
    FLTPT* m_dayLen;
    /// solar radiation for the day (MJ/m2)
    FLTPT* m_sr;


    FLTPT* m_qRchOut;    ///< channel outflow
    FLTPT* m_chStorage;  ///< reach storage (m^3) at time
    FLTPT* m_rteWtrIn;   ///< Water flowing in reach on day before channel routing, m^3
    FLTPT* m_rteWtrOut;  ///< Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    FLTPT* m_chWtrDepth; ///< channel water depth m
    FLTPT* m_chTemp;     ///< temperature of water in reach (deg C)

    FLTPT* m_bc1; ///< rate constant for biological oxidation of NH3 to NO2 in reach at 20 deg C
    FLTPT* m_bc2; ///< rate constant for biological oxidation of NO2 to NO3 in reach at 20 deg C
    FLTPT* m_bc3; ///< rate constant for biological oxidation of organic N to ammonia in reach at 20 deg C
    FLTPT* m_bc4; ///< rate constant for biological oxidation of organic P to dissolved P in reach at 20 deg C

    FLTPT* m_rs1; ///< local algal settling rate in reach at 20 deg C (m/day)
    FLTPT* m_rs2; ///< benthos source rate for dissolved phosphorus in reach at 20 deg C (mg disP-P)/((m**2)*day)
    FLTPT* m_rs3; ///< benthos source rate for ammonia nitrogen in reach at 20 deg C (mg NH4-N)/((m**2)*day)
    FLTPT* m_rs4; ///< rate coefficient for organic nitrogen settling in reach at 20 deg C (1/day)
    FLTPT* m_rs5; ///< organic phosphorus settling rate in reach at 20 deg C (1/day)

    FLTPT* m_rk1; ///< CBOD deoxygenation rate coefficient in reach at 20 deg C (1/day)
    FLTPT* m_rk2; ///< reaeration rate in accordance with Fickian diffusion in reach at 20 deg C (1/day)
    FLTPT* m_rk3; ///< rate of loss of CBOD due to settling in reach at 20 deg C (1/day)
    FLTPT* m_rk4; ///< sediment oxygen demand rate in reach at 20 deg C (mg O2/ ((m**2)*day))

    /// Channel organic nitrogen concentration in basin, ppm
    FLTPT m_chOrgNCo;
    /// Channel organic phosphorus concentration in basin, ppm
    FLTPT m_chOrgPCo;
    /// amount of nitrate transported with lateral flow
    FLTPT* m_latNO3ToCh;
    /// amount of nitrate transported with surface runoff
    FLTPT* m_surfRfNO3ToCh;
    /// amount of ammonian transported with surface runoff
    FLTPT* m_surfRfNH4ToCh;
    /// amount of soluble phosphorus in surface runoff
    FLTPT* m_surfRfSolPToCh;
    /// cod to reach in surface runoff (kg)
    FLTPT* m_surfRfCodToCh;
    /// nitrate loading to reach in groundwater
    FLTPT* m_gwNO3ToCh;
    /// soluble P loading to reach in groundwater
    FLTPT* m_gwSolPToCh;
    // amount of organic nitrogen in surface runoff
    FLTPT* m_surfRfSedOrgNToCh;
    // amount of organic phosphorus in surface runoff
    FLTPT* m_surfRfSedOrgPToCh;
    // amount of active mineral phosphorus absorbed to sediment in surface runoff
    FLTPT* m_surfRfSedAbsorbMinPToCh;
    // amount of stable mineral phosphorus absorbed to sediment in surface runoff
    FLTPT* m_surfRfSedSorbMinPToCh;
    /// amount of ammonium transported with lateral flow
    //FLTPT *m_nh4ToCh;
    /// amount of nitrite transported with lateral flow
    FLTPT* m_no2ToCh;

    /// point source loadings (kg) to channel of each timestep
    /// nitrate
    FLTPT* m_ptNO3ToCh;
    /// ammonia nitrogen
    FLTPT* m_ptNH4ToCh;
    /// Organic nitrogen
    FLTPT* m_ptOrgNToCh;
    /// total nitrogen
    FLTPT* m_ptTNToCh;
    /// soluble (dissolved) phosphorus
    FLTPT* m_ptSolPToCh;
    /// Organic phosphorus
    FLTPT* m_ptOrgPToCh;
    /// total phosphorus
    FLTPT* m_ptTPToCh;
    /// COD
    FLTPT* m_ptCODToCh;

    /// channel erosion
    FLTPT* m_rchDeg;

    /// nutrient amount stored in reach
    /// algal biomass storage in reach (kg)
    FLTPT* m_chAlgae;
    /// organic nitrogen storage in reach (kg)
    FLTPT* m_chOrgN;
    /// ammonia storage in reach (kg)
    FLTPT* m_chNH4;
    /// nitrite storage in reach (kg)
    FLTPT* m_chNO2;
    /// nitrate storage in reach (kg)
    FLTPT* m_chNO3;
    /// total nitrogen in reach (kg)
    FLTPT* m_chTN;
    /// organic phosphorus storage in reach (kg)
    FLTPT* m_chOrgP;
    /// dissolved phosphorus storage in reach (kg)
    FLTPT* m_chSolP;
    /// total phosphorus storage in reach (kg)
    FLTPT* m_chTP;
    /// carbonaceous oxygen demand in reach (kg)
    FLTPT* m_chCOD;
    /// dissolved oxygen storage in reach (kg)
    FLTPT* m_chDOx;
    /// chlorophyll-a storage in reach (kg)
    FLTPT* m_chChlora;
    // saturation storage of dissolved oxygen (kg)
    FLTPT m_chSatDOx;

    /// Outputs, both amount (kg) and concentration (mg/L)
    /// algal biomass amount in reach (kg)
    FLTPT* m_chOutAlgae;
    /// algal biomass concentration in reach (mg/L)
    FLTPT* m_chOutAlgaeConc;
    /// chlorophyll-a biomass amount in reach (kg)
    FLTPT* m_chOutChlora;
    /// chlorophyll-a biomass concentration in reach (mg/L)
    FLTPT* m_chOutChloraConc;
    /// organic nitrogen amount in reach (kg)
    FLTPT* m_chOutOrgN;
    /// organic nitrogen concentration in reach (mg/L)
    FLTPT* m_chOutOrgNConc;
    /// organic phosphorus amount in reach (kg)
    FLTPT* m_chOutOrgP;
    /// organic phosphorus concentration in reach (mg/L)
    FLTPT* m_chOutOrgPConc;
    /// ammonia amount in reach (kg)
    FLTPT* m_chOutNH4;
    /// ammonia concentration in reach (mg/L)
    FLTPT* m_chOutNH4Conc;
    /// nitrite amount in reach (kg)
    FLTPT* m_chOutNO2;
    /// nitrite concentration in reach (mg/L)
    FLTPT* m_chOutNO2Conc;
    /// nitrate amount in reach (kg)
    FLTPT* m_chOutNO3;
    /// nitrate concentration in reach (mg/L)
    FLTPT* m_chOutNO3Conc;
    /// dissolved phosphorus amount in reach (kg)
    FLTPT* m_chOutSolP;
    /// dissolved phosphorus concentration in reach (mg/L)
    FLTPT* m_chOutSolPConc;
    /// carbonaceous oxygen demand in reach (kg)
    FLTPT* m_chOutCOD;
    /// carbonaceous oxygen demand concentration in reach (mg/L)
    FLTPT* m_chOutCODConc;
    /// dissolved oxygen amount in reach (kg)
    FLTPT* m_chOutDOx;
    /// dissolved oxygen concentration in reach (mg/L)
    FLTPT* m_chOutDOxConc;
    /// total N amount in reach (kg)
    FLTPT* m_chOutTN;
    /// total N concentration in reach (mg/L)
    FLTPT* m_chOutTNConc;
    /// total P amount in reach (kg)
    FLTPT* m_chOutTP;
    /// total P concentration in reach (mg/L)
    FLTPT* m_chOutTPConc;

    //intermediate variables

    /// mean day length of each channel (hr)
    FLTPT* m_chDaylen;
    /// mean solar radiation of each channel
    FLTPT* m_chSr;
    /// valid cell numbers of each channel
    int* m_chCellCount;
};

#endif /* SEIMS_MODULE_NUTRCH_QUAL2E_H */
