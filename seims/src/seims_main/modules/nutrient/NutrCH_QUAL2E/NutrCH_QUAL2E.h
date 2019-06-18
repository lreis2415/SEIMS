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

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float data) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

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
    * \return float
    */
    float corTempc(float r20, float thk, float tmp);

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
    float* m_reachDownStream;
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

    float m_ai0; ///< ratio of chlorophyll-a to algal biomass (ug chla/mg alg)
    float m_ai1; ///< fraction of algal biomass that is nitrogen (mg N/mg alg)
    float m_ai2; ///< fraction of algal biomass that is phosphorus (mg P/mg alg)
    float m_ai3; ///< the rate of oxygen production per unit of algal photosynthesis (mg O2/mg alg)
    float m_ai4; ///< the rate of oxygen uptake per unit of algae respiration (mg O2/mg alg)
    float m_ai5; ///< the rate of oxygen uptake per unit of NH3 nitrogen oxidation (mg O2/mg N)
    float m_ai6; ///< the rate of oxygen uptake per unit of NO2 nitrogen oxidation (mg O2/mg N)

    float m_lambda0; ///< non-algal portion of the light extinction coefficient
    float m_lambda1; ///< linear algal self-shading coefficient
    float m_lambda2; ///< nonlinear algal self-shading coefficient

    float m_k_l;   ///< half saturation coefficient for light (MJ/(m2*hr))
    float m_k_n;   ///< half-saturation constant for nitrogen (mg N/L)
    float m_k_p;   ///< half saturation constant for phosphorus (mg P/L)
    float m_p_n;   ///< algal preference factor for ammonia
    float tfact;   ///< fraction of solar radiation computed in the temperature heat balance
                   ///< that is photo synthetically active
    float m_rnum1; ///< fraction of overland flow
    /// option for calculating the local specific growth rate of algae
    //     1: multiplicative:     u = mumax * fll * fnn * fpp
    //     2: limiting nutrient: u = mumax * fll * Min(fnn, fpp)
    //     3: harmonic mean: u = mumax * fll * 2. / ((1/fnn)+(1/fpp))
    int igropt;
    /// maximum specific algal growth rate at 20 deg C
    float m_mumax;
    /// algal respiration rate at 20 deg C (1/day)
    float m_rhoq;
    /// Conversion factor
    float m_cod_n;
    /// Reaction coefficient
    float m_cod_k;

    /// stream link
    float* m_rchID;
    /// soil temperature (deg C)
    float* m_soilTemp;
    /// day length for current day (h)
    float* m_dayLen;
    /// solar radiation for the day (MJ/m2)
    float* m_sr;


    float* m_qRchOut;    ///< channel outflow
    float* m_chStorage;  ///< reach storage (m^3) at time
    float* m_rteWtrIn;   ///< Water flowing in reach on day before channel routing, m^3
    float* m_rteWtrOut;  ///< Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    float* m_chWtrDepth; ///< channel water depth m
    float* m_chTemp;     ///< temperature of water in reach (deg C)

    float* m_bc1; ///< rate constant for biological oxidation of NH3 to NO2 in reach at 20 deg C
    float* m_bc2; ///< rate constant for biological oxidation of NO2 to NO3 in reach at 20 deg C
    float* m_bc3; ///< rate constant for biological oxidation of organic N to ammonia in reach at 20 deg C
    float* m_bc4; ///< rate constant for biological oxidation of organic P to dissolved P in reach at 20 deg C

    float* m_rs1; ///< local algal settling rate in reach at 20 deg C (m/day)
    float* m_rs2; ///< benthos source rate for dissolved phosphorus in reach at 20 deg C (mg disP-P)/((m**2)*day)
    float* m_rs3; ///< benthos source rate for ammonia nitrogen in reach at 20 deg C (mg NH4-N)/((m**2)*day)
    float* m_rs4; ///< rate coefficient for organic nitrogen settling in reach at 20 deg C (1/day)
    float* m_rs5; ///< organic phosphorus settling rate in reach at 20 deg C (1/day)

    float* m_rk1; ///< CBOD deoxygenation rate coefficient in reach at 20 deg C (1/day)
    float* m_rk2; ///< reaeration rate in accordance with Fickian diffusion in reach at 20 deg C (1/day)
    float* m_rk3; ///< rate of loss of CBOD due to settling in reach at 20 deg C (1/day)
    float* m_rk4; ///< sediment oxygen demand rate in reach at 20 deg C (mg O2/ ((m**2)*day))

    /// Channel organic nitrogen concentration in basin, ppm
    float m_chOrgNCo;
    /// Channel organic phosphorus concentration in basin, ppm
    float m_chOrgPCo;
    /// amount of nitrate transported with lateral flow
    float* m_latNO3ToCh;
    /// amount of nitrate transported with surface runoff
    float* m_surfRfNO3ToCh;
    /// amount of ammonian transported with surface runoff
    float* m_surfRfNH4ToCh;
    /// amount of soluble phosphorus in surface runoff
    float* m_surfRfSolPToCh;
    /// cod to reach in surface runoff (kg)
    float* m_surfRfCodToCh;
    /// nitrate loading to reach in groundwater
    float* m_gwNO3ToCh;
    /// soluble P loading to reach in groundwater
    float* m_gwSolPToCh;
    // amount of organic nitrogen in surface runoff
    float* m_surfRfSedOrgNToCh;
    // amount of organic phosphorus in surface runoff
    float* m_surfRfSedOrgPToCh;
    // amount of active mineral phosphorus absorbed to sediment in surface runoff
    float* m_surfRfSedAbsorbMinPToCh;
    // amount of stable mineral phosphorus absorbed to sediment in surface runoff
    float* m_surfRfSedSorbMinPToCh;
    /// amount of ammonium transported with lateral flow
    //float *m_nh4ToCh;
    /// amount of nitrite transported with lateral flow
    float* m_no2ToCh;

    /// point source loadings (kg) to channel of each timestep
    /// nitrate
    float* m_ptNO3ToCh;
    /// ammonia nitrogen
    float* m_ptNH4ToCh;
    /// Organic nitrogen
    float* m_ptOrgNToCh;
    /// total nitrogen
    float* m_ptTNToCh;
    /// soluble (dissolved) phosphorus
    float* m_ptSolPToCh;
    /// Organic phosphorus
    float* m_ptOrgPToCh;
    /// total phosphorus
    float* m_ptTPToCh;
    /// COD
    float* m_ptCODToCh;

    /// channel erosion
    float* m_rchDeg;

    /// nutrient amount stored in reach
    /// algal biomass storage in reach (kg)
    float* m_chAlgae;
    /// organic nitrogen storage in reach (kg)
    float* m_chOrgN;
    /// ammonia storage in reach (kg)
    float* m_chNH4;
    /// nitrite storage in reach (kg)
    float* m_chNO2;
    /// nitrate storage in reach (kg)
    float* m_chNO3;
    /// total nitrogen in reach (kg)
    float* m_chTN;
    /// organic phosphorus storage in reach (kg)
    float* m_chOrgP;
    /// dissolved phosphorus storage in reach (kg)
    float* m_chSolP;
    /// total phosphorus storage in reach (kg)
    float* m_chTP;
    /// carbonaceous oxygen demand in reach (kg)
    float* m_chCOD;
    /// dissolved oxygen storage in reach (kg)
    float* m_chDOx;
    /// chlorophyll-a storage in reach (kg)
    float* m_chChlora;
    // saturation storage of dissolved oxygen (kg)
    float m_chSatDOx;

    /// Outputs, both amount (kg) and concentration (mg/L)
    /// algal biomass amount in reach (kg)
    float* m_chOutAlgae;
    /// algal biomass concentration in reach (mg/L)
    float* m_chOutAlgaeConc;
    /// chlorophyll-a biomass amount in reach (kg)
    float* m_chOutChlora;
    /// chlorophyll-a biomass concentration in reach (mg/L)
    float* m_chOutChloraConc;
    /// organic nitrogen amount in reach (kg)
    float* m_chOutOrgN;
    /// organic nitrogen concentration in reach (mg/L)
    float* m_chOutOrgNConc;
    /// organic phosphorus amount in reach (kg)
    float* m_chOutOrgP;
    /// organic phosphorus concentration in reach (mg/L)
    float* m_chOutOrgPConc;
    /// ammonia amount in reach (kg)
    float* m_chOutNH4;
    /// ammonia concentration in reach (mg/L)
    float* m_chOutNH4Conc;
    /// nitrite amount in reach (kg)
    float* m_chOutNO2;
    /// nitrite concentration in reach (mg/L)
    float* m_chOutNO2Conc;
    /// nitrate amount in reach (kg)
    float* m_chOutNO3;
    /// nitrate concentration in reach (mg/L)
    float* m_chOutNO3Conc;
    /// dissolved phosphorus amount in reach (kg)
    float* m_chOutSolP;
    /// dissolved phosphorus concentration in reach (mg/L)
    float* m_chOutSolPConc;
    /// carbonaceous oxygen demand in reach (kg)
    float* m_chOutCOD;
    /// carbonaceous oxygen demand concentration in reach (mg/L)
    float* m_chOutCODConc;
    /// dissolved oxygen amount in reach (kg)
    float* m_chOutDOx;
    /// dissolved oxygen concentration in reach (mg/L)
    float* m_chOutDOxConc;
    /// total N amount in reach (kg)
    float* m_chOutTN;
    /// total N concentration in reach (mg/L)
    float* m_chOutTNConc;
    /// total P amount in reach (kg)
    float* m_chOutTP;
    /// total P concentration in reach (mg/L)
    float* m_chOutTPConc;

    //intermediate variables

    /// mean day length of each channel (hr)
    float* m_chDaylen;
    /// mean solar radiation of each channel
    float* m_chSr;
    /// valid cell numbers of each channel
    int* m_chCellCount;
};

#endif /* SEIMS_MODULE_NUTRCH_QUAL2E_H */
