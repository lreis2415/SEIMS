/*!
 * \file NUTR_CH.h
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 * \author Hui Wu
 * \date Jul. 2012
 * \revised LiangJun Zhu
 * \date May/ 2016
 */

#pragma once

#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include "SimulationModule.h"

using namespace std;
/** \defgroup NUTR_CH
 * \ingroup Erosion
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 */
/*!
 * \class NUTR_CH
 * \ingroup NUTR_CH
 *
 * \brief Sediment routing using variable channel dimension(VCD) method at daily time scale
 *
 */
class NUTR_CH : public SimulationModule
{
public:
    //! Constructor
    NUTR_CH(void);

    //! Destructor
    ~NUTR_CH(void);

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

    virtual TimeStepType GetTimeStepType() { return TIMESTEP_CHANNEL; };
private:
    /// time step (sec)
    int m_dt;
    /// reach number (= subbasin number)
    int m_nreach;

    /// diversion loss (Vdiv) of the river reach, m_Vid[id], id is the reach id
    float *m_Vdiv;
    /// The point source discharge, m_Vpoint[id], id is the reach id
    float *m_Vpoint;

    /// initial channel storage per meter of reach length (m3/m)
    float m_Chs0;

    /// channel outflow
    float *m_qchOut;

    float *m_chOrder;
    float *m_chWidth;
    float *m_chDepth;
    float *m_chLen;      //length of reach (m)


    /// downstream id (The value is 0 if there if no downstream reach)
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;

    // id the reaches
    float *m_reachId;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;
    map<int, vector<int> > m_reachLayers;

    /// reach storage (m3) at time t
    float *m_chStorage;

	// input nutrient from hillslope
	float *m_latno3ToCh;  // amount of nitrate transported with lateral flow to channel
	float *m_sur_no3ToCh; // amount of nitrate transported with surface runoff to channel
	float *m_sur_solpToCh;// amount of soluble phosphorus in surface runoff to channel
	float *m_sedorgnToCh;  // amount of organic N in surface runoff to channel
	float *m_sedorgpToCh;  // amount of organic P in surface runoff to channel
	float *m_no3gwToCh;    // nitrate loading to reach in groundwater
	float *m_minpgwToCh;   // soluble P loading to reach in groundwater

	//output
	float *m_chOrgN;    // organic nitrogen concentration in stream (mg N/L)
	float *m_chNo3;     // NO3 concentration in stream (mg/L)
	float *m_chOrgP;    // organic P concentration in stream (mg/L)
	float *m_chSolP;    // solute P concentration in stream (mg/L)

    void initialOutputs();
    void SedChannelRouting(int i);

};

