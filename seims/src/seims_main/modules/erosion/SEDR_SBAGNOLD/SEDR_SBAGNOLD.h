/*!
 * \brief Sediment routing using simplified version of Bagnold(1997) stream power equation
 *        which is also the original SWAT method.
 *        This module routes sediment from subbasin to basin outlet deposition is based on fall velocity and degradation on stream
 *        reWrite from route.f and rtsed.f of SWAT
 * \author Hui Wu
 * \date Jul. 2012
 * \revised LiangJun Zhu
 * \date May. 2016
 * \description: 1. move m_erodibilityFactor, m_coverFactor, to reach collection of MongoDB as inputs, and is DT_Array1D
 *               2. add point source loadings from Scenario database
 *               3. add SEDRECHConc output with the unit g/cm3 (i.e., Mg/m3)
 * \revised Junzhi Liu
 * \date August. 2016
 * \revised LiangJun Zhu
 * \date Sep. 2016
 * \description: 1. ReCheck and Update code according to route.f and rtsed.f
 *               2. Change the module name from SEDR_VCD to SEDR_SBAGNOLD
 */
#ifndef SEIMS_MODULE_SEDR_SBAGNOLD_H
#define SEIMS_MODULE_SEDR_SBAGNOLD_H

#include "SimulationModule.h"

using namespace std;
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
class SEDR_SBAGNOLD : public SimulationModule {
public:
    //! Constructor
    SEDR_SBAGNOLD();

    //! Destructor
    ~SEDR_SBAGNOLD();

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    virtual void SetReaches(clsReaches *reaches);

    virtual void SetScenario(Scenario *sce);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData();

    virtual TimeStepType GetTimeStepType() { return TIMESTEP_CHANNEL; };
private:
    /// time step (sec)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;
    /// layering method, 0 means UP_DOWN, 1 means DOWN_UP
    LayeringMethod m_layeringMethod;
    /// whether change channel dimensions, 0 - do not change, 1 - compute channel degredation
    int m_VCD;
    /// the peak rate adjustment factor
    float m_prf;
    /// Coefficient in sediment transport equation
    float m_spcon;
    /// Exponent in sediment transport equation
    float m_spexp;
    /// critical velocity for sediment deposition
    float m_vcrit;

    /// sediment from subbasin (hillslope), kg
    float *m_sedtoCh;
    /// cross-sectional area of flow in the channel (m^2)
    float *m_CrAreaCh;
    /// initial channel storage per meter of reach length (m^3/m)
    float m_Chs0;
    /// Initial channel sediment concentration, ton/m^3, i.e., kg/L
    float m_sedChi0;
    /// channel outflow, m^3/s
    float *m_qchOut;

    float *m_chOrder;
    float *m_chWidth;
    float *m_chDepth;
    //length of reach (m)
    float *m_chLen;
    float *m_chVel;
    float *m_chSlope;
    float *m_chManning;
    /// reach cover factor
    float *m_chCover;
    /// channel erodibility factor (cm/hr/Pa)
    float *m_chErod;
    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
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
    float *m_ptSub;
    /// reach storage (m^3) at time t
    float *m_chStorage;
    /// reach storage of previous timestep, m^3
    float *m_preChStorage;
    /// channel water depth, m
    float *m_chWTdepth;
    /// channel water depth of previous timestep, m
    float *m_preChWTDepth;
    /// channel bankfull width, m
    float *m_chWTWidth;
    // OUTPUT
    /// initial reach sediment out (kg) at time t
    float *m_sedOut;
    /// channel sediment storage (kg)
    float *m_sedStorage;
    /// sediment of deposition
    float *m_sedDep;
    /// sediment of degradation
    float *m_sedDeg;
    /// sediment concentration (g/L, i.e., kg/m3)
    float *m_sedConc;
    float *m_rchSand;
    float *m_rchSilt;
    float *m_rchClay;
    float *m_rchSag;
    float *m_rchLag;
    float *m_rchGra;

    float *m_rchBankEro;
    float *m_rchDeg;
    float *m_rchDep;
    float *m_flplainDep;

    void initialOutputs();

    void PointSourceLoading();

    void SedChannelRouting(int i);

    void doChannelDowncuttingAndWidening(int id);
};

#endif /* SEIMS_MODULE_SEDR_SBAGNOLD_H */