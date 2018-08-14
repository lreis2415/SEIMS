/*!
 * \brief channel flow routing using Muskingum method
 *        Refers to rtmusk.f of SWAT source.
 * \author Junzhi Liu
 * \changelog  2012-06-26 - jz - Initial implementation.\n
 *             2016-09-18 - lj - 1. Add point source loadings from Scenario.\n
 *                               2. Assume the channels have a trapezoidal shape.\n
 *                               3. Add m_chBtmWidth as variable intermediate parameter.\n
 *                               4. Add m_chSideSlope (default is 2) as input parameter from MongoDB,
 *                                  which is the ratio of run to rise.\n
 *                               5. Add several variables to store values in previous time step,
 *                                  which will be use in QUAL2E etc.\n
 *             2018-03-16 - lj - 1. Use AddInOutput() to solve the passing data across subbasins for MPI version.\n
 *                               2. Code style review.\n
 *             2018-08-11 - lj - Updates the calculation of Muskingum weights according to SWAT.\n
 */
#ifndef SEIMS_MODULE_MUSK_CH_H
#define SEIMS_MODULE_MUSK_CH_H

#include "SimulationModule.h"
#include "Scenario.h"

using namespace bmps;

/** \defgroup MUSK_CH
 * \ingroup Hydrology_longterm
 * \brief channel flow routing using Muskingum method
 */

///*
// * \struct MuskWeights coefficients
// */
//struct MuskWeights {
//    float c1;
//    float c2;
//    float c3;
//    float c4;
//    float dt;
//    int n; ///< number of division of the origin time step
//};

/*!
 * \class MUSK_CH
 * \ingroup MUSK_CH
 * \brief channel flow routing using Muskingum method
 *
 */
class MUSK_CH: public SimulationModule {
public:
    MUSK_CH();

    virtual ~MUSK_CH();

    int Execute() OVERRIDE;

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float value) OVERRIDE;

    void GetValue(const char* key, float* value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    bool CheckInputSize(const char* key, int n);

    bool CheckInputData();

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

private:
    void InitialOutputs();

    void PointSourceLoading();

    bool ChannelFlow(int i);

    /*!
     * \brief Calculates flow rate or flow velocity using Manning's
     *        equation. If x1 is set to 1, the velocity is calculated. If x1 is set to
     *        cross-sectional area of flow, the flow rate is calculated.
     * \param[in] x1 cross-sectional flow area or 1, m^2 or none.
     * \param[in] x2 hydraulic radius, m.
     * \param[in] x3 Manning's "n" value for channel.
     * \param[in] x4 average slope of channel, m/m.
     * \return flow rate or flow velocity, m^3/s or m/s.
     */
    //float manningQ(float x1, float x2, float x3, float x4);

    //static void GetDt(float timeStep, float fmin, float fmax, float& dt, int& n);

    //void GetCoefficients(float reachLength, float v0, MuskWeights& weights);

    //void updateWaterWidthDepth(int i);

    // void updateChannleBottomWidth(int i);
private:
    /// time step (sec)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    /// outlet ID, also can be derived by m_reachLayers.rbegin()->second[0];
    int m_outletID;
    /// The point source discharge (m3/s), m_ptSub[id], id is the reach id, load from m_Scenario
    float* m_ptSub;

    /// Manning's "n" value for the main channel
    float* m_chMan;
    /// average slope of main channel
    float* m_chSlope;
    /// hydraulic conductivity of the channel bed (mm/h)
    float* m_Kchb;
    /// hydraulic conductivity of the channel bank (mm/h)
    float* m_Kbank;
    /// reach evaporation adjustment factor, evrch in SWAT.
    /// Evaporation from the reach is multiplied by evrch.
    /// This variable was created to limit the evaporation predicted in arid regions.
    float m_Epch;
    /// initial bank storage per meter of reach length (m3/m)
    float m_Bnk0;
    /// initial channel storage per meter of reach length (m3/m)
    //float m_Chs0;

    /// inverse of the channel side slope, by default is 2. chside in SWAT.
    float* m_chSideSlope;
    /// initial percentage of channel volume
    float m_Chs0_perc;
    /// the initial volume of transmission loss to the deep aquifer over the time interval (m3/s)
    //float m_Vseep0; // useless?
    /// bank flow recession constant
    float m_aBank;
    /// bank storage loss coefficient
    float m_bBank;
    ///subbasin grid
    float* m_subbsnID;
    /// the subbasin area (m2)  //add to the reach parameters file
    float* m_area;

    /// Average PET for each subbasin, mm
    float* m_petCh;

    /// overland flow to streams from each subbasin (m3/s)
    float* m_qsSub;
    /// interflow to streams from each subbasin (m3/s)
    float* m_qiSub;
    /// groundwater flow out of the subbasin (m3/s)
    float* m_qgSub;
    ///  Groundwater storage (mm) of the subbasin
    float* m_gwSto;

    // channel outflow
    float* m_qsCh; ///< surface part of channel outflow
    float* m_qiCh; ///< subsurface part of channel outflow
    float* m_qgCh; ///< groundwater part of channel outflow
    /// channel width (m)
    float* m_chWth;
    /// channel water width (m), topw in SWAT
    float* m_chWtrWth;
    /// bottom width of channel (m)
    float* m_chBtmWth;
    /// cross-sectional area (m^2)
    float* m_chCrossArea;
    /// channel depth (m)
    float* m_chDepth;
    /// channel water depth (m), rchdep in SWAT
    float* m_chWtrDepth;
    /// channel water depth of previous timestep (m)
    float* m_preChWtrDepth;
    /// channel length (m)
    float* m_chLen;
    /// bank storage (m^3)
    float* m_bankSto;
    /// seepage to deep aquifer
    float* m_seepage;

    /// downstream id (The value is -1 if there if no downstream reach)
    float* m_reachDownStream;
    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;

    // Muskingum input parameters
    // Weighting factor controlling relative importance of inflow rate and outflow rate in determining water storage in reach segment
    float m_mskX;
    // Calibration coefficient used to control impact of the storage time constant for normal flow
    float m_mskCoef1;
    // Calibration coefficient used to control impact of the storage time constant fro low flow
    float m_mskCoef2;
    /// scenario data

    /* point source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: point source management factory instance
     */
    map<int, BMPPointSrcFactory *> m_ptSrcFactory;
    //temporary at routing time

    /// flowin discharge at the last time step
    //float* m_qIn;
    /// flow into reach for routing iteration, m^3
    float* m_flowIn;
    /// flow out of reach for routing iteration, m^3
    float* m_flowOut;
    /// Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    float* m_rteWtrOut;
    /// reach storage (m^3) at time, t
    float* m_chSto;
    /// reach storage (m^3) at previous time step, t-1
    float* m_preChSto;
    /// reach outflow (m3/s) at time, t
    float* m_qRchOut;
    /*!
     * reach layers
     * key: computing order, \sa LayeringMethod
     * value: reach ID
     */
    map<int, vector<int> > m_rteLyrs;
};

#endif /* SEIMS_MODULE_MUSK_CH_H */
