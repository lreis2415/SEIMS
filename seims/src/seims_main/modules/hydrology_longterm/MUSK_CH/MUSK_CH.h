/*!
 * \brief channel flow routing using Muskingum method
 * \author Junzhi Liu
 * \version 1.0
 * \date 26-Jul-2012
 * 
 * \revision Liangjun Zhu
 * \date 18-Sep-2016
 * \description: 1. Add point source loadings from Scenario.
 *               2. Assume the channels have a trapezoidal shape
 *               3. Add m_chBtmWidth as variable intermediate parameter
 *               4. Add m_chSideSlope (default is 2) as input parameter from MongoDB, which is the ratio of run to rise
 *               5. Add several variables to store values in previous time step, which will be use in QUAL2E etc.
 */
#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include <cmath>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm> 
#include "SimulationModule.h"
#include "Scenario.h"
using namespace std;
using namespace MainBMP;
/** \defgroup MUSK_CH
 * \ingroup Hydrology_longterm
 * \brief channel flow routing using Muskingum method
 *
 */

/* 
 * \struct MuskWeights coefficients
 */
struct MuskWeights
{
    float c1;
    float c2;
    float c3;
    float c4;
    float dt;
    int n;  ///< number of division of the origin time step
};

/*!
 * \class MUSK_CH
 * \ingroup MUSK_CH
 * \brief channel flow routing using Muskingum method
 *
 */
class MUSK_CH : public SimulationModule
{
public:
    MUSK_CH(void);

    ~MUSK_CH(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    //virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

	virtual void SetScenario(Scenario *sce);

	virtual void SetReaches(clsReaches *reaches);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputSizeChannel(const char *key, int n);

    bool CheckInputData(void);

    virtual TimeStepType GetTimeStepType() {return TIMESTEP_CHANNEL;};

private:
	//! 
    float m_vScalingFactor;

    /// time step (sec)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;
	/// outlet ID
	int m_outletID;
    /// The point source discharge (m3/s), m_ptSub[id], id is the reach id, load from m_Scenario
    float *m_ptSub;

    /// hydraulic conductivity of the channel bed (mm/h)
    float m_Kchb;
    /// hydraulic conductivity of the channel bank (mm/h)
    float m_Kbank;
    /// reach evaporation adjustment factor;
    float m_Epch;
    /// initial bank storage per meter of reach length (m3/m)
    float m_Bnk0;
    /// initial channel storage per meter of reach length (m3/m)
    //float m_Chs0;
	
	/// inverse of the channel side slope, by default is 2.
	float *m_chSideSlope;
	/// initial percentage of channel volume
	float m_Chs0_perc;
    /// the initial volume of transmission loss to the deep aquifer over the time interval (m3/s)
    float m_Vseep0;
    /// bank flow recession constant
    float m_aBank;
    /// bank storage loss coefficient
    float m_bBank;
	///subbasin grid
    float *m_subbasin;                
    /// the subbasin area (m2)  //add to the reach parameters file
    float *m_area;

    /// Average PET for each subbasin
    float *m_petCh;

    /// overland flow to streams from each subbasin (m3/s)
    float *m_qsSub;
    /// interflow to streams from each subbasin (m3/s)
    float *m_qiSub;
    /// groundwater flow out of the subbasin (m3/s)
    float *m_qgSub;
    ///  Groundwater storage (mm) of the subbasin
    float *m_gwStorage;

    /// channel outflow
    float *m_qsCh;
    float *m_qiCh;
    float *m_qgCh;
	/// channel order
    float *m_chOrder;
	/// channel width (m)
    float *m_chWidth;
	/// channel water width (m)
	float *m_chWTWidth;
	/// bottom width of channel (m)
	float *m_chBtmWidth;
	/// channel depth (m)
    float *m_chDepth;
	/// channel water depth (m)
	float *m_chWTdepth;
	/// channel water depth of previous timestep (m)
	float *m_preChWTDepth;
	/// channel length (m)
    float *m_chLen;
	/// channel flow velocity (m/s)
    float *m_chVel;
	/// bank storage (m^3)
    float *m_bankStorage;
	/// groundwater recharge to channel or perennial base flow, m^3/s
    float m_deepGroundwater;

    /// seepage to deep aquifer
    float *m_seepage;

    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;

    // the reaches id 
    //float *m_reachId;
	vector<int> m_reachId;
    // for muskingum
    float m_x;
    float m_co1;
	// IS THIS USEFUL? BY LJ
    float m_qUpReach;
	/// scenario data

	/* point source operations
	 * key: unique index, BMPID * 100000 + subScenarioID
	 * value: point source management factory instance
	 */
	map<int, BMPPointSrcFactory*> m_ptSrcFactory;
    //temporary at routing time

    /// reach storage (m^3) at time, t
    float *m_chStorage;
	/// reach storage (m^3) at previous time step, t-1
	float *m_preChStorage;
    /// reach outflow (m3/s) at time, t
    float *m_qOut;
    /// flowin discharge at the last time step
    float *m_qIn;
	/*
	 * reach layers
	 * key: stream order
	 * value: reach ID
	 */
    map<int, vector<int> > m_reachLayers;

    void initialOutputs();

	void PointSourceLoading();

    void ChannelFlow(int i);

    void GetDt(float timeStep, float fmin, float fmax, float &dt, int &n);

    void GetCoefficients(float reachLength, float v0, MuskWeights &weights);

	void updateWaterWidthDepth(int i);
};

