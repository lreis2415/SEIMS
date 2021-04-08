/*!
 * \file MUSK_CH.h
 * \brief channel flow routing using Muskingum method
 *        Refers to rtmusk.f of SWAT source.
 *
 * Changelog:
 *   - 1. 2012-06-26 - jz - Initial implementation.
 *   - 2. 2016-09-18 - lj -
 *        -# Add point source loadings from Scenario.
 *        -# Assume the channels have a trapezoidal shape.
 *        -# Add m_chBtmWidth as variable intermediate parameter.
 *        -# Add m_chSideSlope (default is 2) as input parameter from MongoDB,
 *             which is the ratio of run to rise.
 *        -# Add several variables to store values in previous time step,
 *             which will be use in QUAL2E etc.
 *   - 3. 2018-03-16 - lj - Use AddInOutput() to solve the passing data across
 *                            subbasins for MPI version. And code style review.
 *   - 4. 2018-08-14 - lj - Updates according to SWAT.
 *
 * \author Liangjun Zhu, Junzhi Liu
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

    void SetValue(const char* key, float value) OVERRIDE;

    void SetValueByIndex(const char* key, int index, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    TimeStepType GetTimeStepType() OVERRIDE { return TIMESTEP_CHANNEL; }

    void GetValue(const char* key, float* value) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:

    void PointSourceLoading();

    bool ChannelFlow(int i);

private:
    int m_dt;            ///< time step (sec)
    int m_inputSubbsnID; ///< current subbasin ID, 0 for the entire watershed

    int m_nreach;      ///< reach number (= subbasin number)
    int m_outletID;    ///< outlet ID, also can be derived by m_reachLayers.rbegin()->second[0];
    float m_Epch;      ///< reach evaporation adjustment factor, evrch in SWAT.
    float m_Bnk0;      ///< initial bank storage per meter of reach length (m^3/m)
    float m_Chs0_perc; ///< initial percentage of channel water volume
    float m_aBank;     ///< bank flow recession constant
    float m_bBank;     ///< bank storage loss coefficient
    float* m_subbsnID; ///< Subbasin grid

    /// Muskingum input parameters

    // Weighting factor controlling relative importance of inflow rate and outflow rate in determining water storage in reach segment
    float m_mskX;
    // Calibration coefficient used to control impact of the storage time constant for normal flow
    float m_mskCoef1;
    // Calibration coefficient used to control impact of the storage time constant fro low flow
    float m_mskCoef2;

    /// Reach information

    float* m_chWth;           ///< channel width (m)
    float* m_chDepth;         ///< channel depth (m)
    float* m_chLen;           ///< channel length (m)
    float* m_chArea;          ///< the reach area (m^2) at bankfull
    float* m_chSideSlope;     ///< inverse of the channel side slope, by default is 2. chside in SWAT.
    float* m_chSlope;         ///< average slope of main channel
    float* m_chMan;           ///< Manning's "n" value for the main channel
    float* m_Kchb;            ///< hydraulic conductivity of the channel bed (mm/h)
    float* m_Kbank;           ///< hydraulic conductivity of the channel bank (mm/h)
    float* m_reachDownStream; ///< downstream id (The value is -1 if there if no downstream reach)
    /*!
     * Index of upstream Ids (The value is -1 if there if no upstream reach)
     * m_reachUpStream.size() = N+1
     * m_reachUpStream[1] = [2, 3] means Reach 2 and Reach 3 flow into Reach 1.
     */
    vector<vector<int> > m_reachUpStream;
    /*!
     * reach layers
     * key: computing order, \sa LayeringMethod
     * value: reach ID
     */
    map<int, vector<int> > m_rteLyrs;

    /// scenario data

    /*!
     * point source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: point source management factory instance
     */
    map<int, BMPPointSrcFactory *> m_ptSrcFactory;


    // Inputs from other modules

    float* m_petSubbsn; ///< Average PET of each subbasin, mm
    float* m_gwSto;     ///< Groundwater storage (mm) of the subbasin
    float* m_olQ2Rch;   ///< overland flow to streams from each subbasin (m^3/s)
    float* m_ifluQ2Rch; ///< interflow to streams from each subbasin (m^3/s)
    float* m_gndQ2Rch;  ///< groundwater flow out of the subbasin (m^3/s)

    // Temporary variables

    float* m_ptSub;   ///< The point source discharge (m^3/s) load from m_ptSrcFactory
    float* m_flowIn;  ///< flow into reach for routing iteration, m^3
    float* m_flowOut; ///< flow out of reach for routing iteration, m^3
    float* m_seepage; ///< seepage to deep aquifer

    // Ouputs

    float* m_qRchOut;  ///< reach outflow (m^3/s), sdti in SWAT
    float* m_qsRchOut; ///< surface part of channel outflow
    float* m_qiRchOut; ///< subsurface part of channel outflow
    float* m_qgRchOut; ///< groundwater part of channel outflow

    float* m_chSto;     ///< reach storage (m^3), rchstor in SWAT
    float* m_rteWtrIn;  ///< Water flowing in reach on day before channel routing, m^3
    float* m_rteWtrOut; ///< Water leaving reach on day after channel routing, m^3, rtwtr in SWAT
    float* m_bankSto;   ///< bank storage (m^3), bankst in SWAT

    float* m_chWtrDepth;  ///< channel water depth (m), rchdep in SWAT
    float* m_chWtrWth;    ///< channel water width (m), topw in SWAT
    float* m_chBtmWth;    ///< bottom width of channel (m), phi(6,:) in SWAT
    float* m_chCrossArea; ///< cross-sectional area (m^2), rcharea in SWAT
};

#endif /* SEIMS_MODULE_MUSK_CH_H */
