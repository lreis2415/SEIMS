/*!
 * \brief Sediment routing using simplified version of Bagnold(1997) stream power equation
 *        which is also the original SWAT method.
 *        This module routes sediment from subbasin to basin outlet deposition is based on fall velocity and degradation on stream
 *        reWrite from route.f and rtsed.f of SWAT
 * \author Hui Wu, Liangjun Zhu, Junzhi Liu
 * \changelog 2012-07-31 - wh - Initial implementation.\n
 *            2016-05-30 - lj - 1. move m_erodibilityFactor, m_coverFactor, to reach collection of MongoDB as inputs, and is DT_Array1D.\n
 *                              2. add point source loadings from Scenario database.\n
 *                              3. add SEDRECHConc output with the unit g/cm3 (i.e., Mg/m3).\n
 *            2016-08-31 - jz - Code review.\n
 *            2016-09-30 - lj - 1. ReCheck and Update code according to route.f and rtsed.f\n
 *                              2. Change the module name from SEDR_VCD to SEDR_SBAGNOLD.\n
 *            2018-05-14 - lj - Code review and reformat.\n
 *            2018-06-28 - lj - 1. The initialization of m_sedStorage should be done in Set1DData after m_chStorage.\n
 *                              2. Bug fixed about code related to the IN/OUTPUT variables.\n
 *
 */
#ifndef SEIMS_MODULE_SEDR_SBAGNOLD_H
#define SEIMS_MODULE_SEDR_SBAGNOLD_H

#include "SimulationModule.h"

/** \defgroup SEDR_SBAGNOLD
 * \ingroup Erosion
 * \brief Sediment routing using Simplified version of Bagnold(1997) stream power equation
 */
/*!
 * \class SEDR_SBAGNOLD
 * \ingroup SEDR_SBAGNOLD
 *
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 *
 */
class SEDR_SBAGNOLD: public SimulationModule {
public:
    SEDR_SBAGNOLD();

    ~SEDR_SBAGNOLD();

    int Execute() OVERRIDE;

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    bool CheckInputSize(const char* key, int n);

    bool CheckInputData();

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; };
private:
    void InitialOutputs();

    void PointSourceLoading();

    void SedChannelRouting(int i);

    void DoChannelDowncuttingAndWidening(int id);
private:
    /// time step (sec)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    /// whether change channel dimensions, 0 - do not change (false), 1 - compute channel degredation (true)
    bool m_vcd;
    /// the peak rate adjustment factor
    float m_peakRateAdj;
    /// Coefficient in sediment transport equation, spcon in SWAT
    float m_sedTransEqCoef;
    /// Exponent in sediment transport equation, spexp in SWAT
    float m_sedTransEqExp;
    /// critical velocity for sediment deposition
    float m_critVelSedDep;

    /// sediment from subbasin (hillslope), kg
    float* m_sedtoCh;
    /// cross-sectional area of flow in the channel (m^2), not used!
    /// float *m_CrAreaCh;
    /// initial channel storage per meter of reach length (m^3/m)
    float m_initChStorage;
    /// Initial channel sediment concentration, ton/m^3, i.e., kg/L
    float m_initChSedConc;
    /// channel outflow, m^3/s
    float* m_qRchOut;

    float* m_chOrder;
    float* m_chWidth;
    float* m_chDepth;
    //length of reach (m)
    float* m_chLen;
    float* m_chSlope;
    /// float *m_chManning; not used!
    /// reach cover factor
    float* m_chCover;
    /// channel erodibility factor (cm/hr/Pa)
    float* m_chErod;
    /// downstream id (The value is 0 if there if no downstream reach)
    float* m_reachDownStream;
    /// Reach layers according to \a LayeringMethod
    map<int, vector<int> > m_reachLayers;

    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;

    /* point source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: point source management factory instance
     */
    map<int, BMPPointSrcFactory *> m_ptSrcFactory;
    /// The point source loading (kg), m_ptSub[id], id is the reach id, load from m_Scenario
    float* m_ptSub;
    /// reach storage (m^3) at time t
    float* m_chStorage;
    /// reach storage of previous timestep, m^3
    float* m_preChStorage;
    /// channel water depth, m
    float* m_chWtrDepth;
    /// channel water depth of previous timestep, m
    float* m_preChWtrDepth;
    /// channel bankfull width, m
    float* m_chWtrWth;
    // OUTPUT
    /// reach sediment out (kg) at time t
    float* m_sedRchOut;
    /// channel sediment storage (kg)
    float* m_sedStorage;
    /// sediment of deposition
    float* m_sedDep;
    /// sediment of degradation
    float* m_sedDeg;
    /// sediment concentration (g/L, i.e., kg/m3)
    float* m_sedConcRchOut;
    float* m_rchSand;
    float* m_rchSilt;
    float* m_rchClay;
    float* m_rchSag;
    float* m_rchLag;
    float* m_rchGra;

    float* m_rchBankEro;
    float* m_rchDeg;
    float* m_rchDep;
    float* m_fldPlainDep;
};
#endif /* SEIMS_MODULE_SEDR_SBAGNOLD_H */
