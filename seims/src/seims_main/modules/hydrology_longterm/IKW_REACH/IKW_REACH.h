/*----------------------------------------------------------------------
*	Purpose: 	channel flow routing using Muskingum method
*
*	Created:	Junzhi Liu
*	Date:		26-Jule-2012
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/

#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include "SimulationModule.h"

using namespace std;
/** \defgroup IKW_REACH
 * \ingroup Hydrology_longterm
 * \brief channel flow routing using Muskingum method
 *
 */

/*!
 * \class IKW_REACH
 * \ingroup IKW_REACH
 * \brief Overland routing using 4-point implicit finite difference method
 * 
 */
struct MuskWeights
{
    float c1;
    float c2;
    float c3;
    float c4;
    float dt;
    int n;  // number of division of the origin time step
};

class IKW_REACH : public SimulationModule
{
public:
    IKW_REACH(void);

    ~IKW_REACH(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputSizeChannel(const char *key, int n);

    bool CheckInputData(void);

    virtual TimeStepType GetTimeStepType()
    {
        return TIMESTEP_CHANNEL;
    };

private:
    float m_manningScalingFactor;

    /// time step (hr)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;

    ///// diversion loss (Vdiv) of the river reach .. m_Vid[id], id is the reach id
    //float *m_Vdiv;
    ///// The point source discharge .. m_Vpoint[id], id is the reach id
    //float *m_Vpoint;

    /// hydraulic conductivity of the channel bed (mm/h)
    float m_Kchb;
    /// hydraulic conductivity of the channel bank (mm/h)
    float m_Kbank;
    /// reach evaporation adjustment factor;
    float m_Epch;
    /// initial bank storage per meter of reach length (m3/m)
    float m_Bnk0;
    /// initial channel storage per meter of reach length (m3/m)
    float m_Chs0;
    /// the initial volume of transmission loss to the deep aquifer over the time interval (m3/s)
    float m_Vseep0;   //added
    /// bank flow recession constant
    float m_aBank;
    /// bank storage loss coefficient
    float m_bBank;

    float *m_subbasin;                //subbasin grid
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

    float *m_chOrder;
    float *m_chWidth;
    float *m_chDepth;
    float *m_chLen;
    float *m_chVel;
    float *m_chManning;
    float *m_chSlope;

    float *m_bankStorage;

    float m_deepGroudwater;

    /// seepage to deep aquifer
    float *m_seepage;

    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;

    // id the reaches
    float *m_reachId;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;

    // for muskingum
    float m_x;
    float m_co1;

    float m_qUpReach;

    //temporary at routing time
    /// reach storage (m3) at time t
    float *m_chStorage;

    /// reach outflow (m3/s) at time t
    float *m_qOut;
    /// flowin discharge at the last time step
    float *m_qIn;
    /// channel water depth m
    float *m_chWTdepth;

    map<int, vector<int> > m_reachLayers;

    void initialOutputs();

    void ChannelFlow(int i);

    float GetNewQ(float qIn, float qLast, float surplus, float alpha, float dt, float dx);
};

