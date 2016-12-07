/*!
 * \file KinWavSed_CH.h
 * \brief Kinematic wave method for channel flow erosion and deposition
 * \author Hui Wu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */

#ifndef SEIMS_KinWavSed_CH_INCLUDE
#define SEIMS_KinWavSed_CH_INCLUDE

#include "api.h"
#include <string>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include "SimulationModule.h"

using namespace std;
/** \defgroup KinWavSed_CH
 * \ingroup Erosion
 * \brief Kinematic wave method for channel flow erosion and deposition
 */
/*!
 * \class KinWavSed_CH
 * \ingroup KinWavSed_CH
 *
 * \brief Kinematic wave method for channel flow erosion and deposition
 *
 */
class KinWavSed_CH : public SimulationModule
{
public:
    //! Constructor
    KinWavSed_CH(void);

    //! Destructor
    ~KinWavSed_CH(void);

    //! Execute
    virtual int Execute();

    virtual void SetValue(const char *key, float value);

    virtual void GetValue(const char *key, float *value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);

private:
    //Parameters

    /*/// calibration coefficient
    float m_eco1;
    /// calibration coefficient
    float m_eco2;*/
    /**
    *	@brief reach layers map
    *
    *	The first element in each sub-array is the number of reaches in this sub-array
    *	The following elements are the index of reach array
    */
    map<int, vector<int> > m_reachLayers;
    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;
    /// flow out index
    float *m_flowOutIndex;
    // id the reaches
    float *m_reachId;
    /// channel width (zero for non-channel cells)
    float *m_chWidth;
    /// stream order
    float *m_streamOrder;
    /// stream id of downstream
    float *m_reachDownStream;
    /// upstream id (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;
    /**
    *	@brief reach links
    *
    *	key: index of the reach
    *	value: vector of cell index
    */
    map<int, vector<int> > m_reachs;


    /// cell width of grid map (m)
    float m_CellWith;
    /// number of valid cells
    int m_nCells;
    /// length of time step (s)
    float m_TimeStep;
    ///calibration coefficient of transport capacity
    float m_ChTcCo;
    /// calibration coefficient of channel flow detachment
    float m_ChDetCo;
    /// slope of map, to calculate slope gradient.
    float *m_Slope;
    /// USLE K
    float *m_USLE_K;
    /*/// hydraulic radius (m)
    float* m_R;*/
    /// id of source cells of reaches
    int *m_sourceCellIds;
    /// channel number
    int m_chNumber;
    /// stream link
    float *m_streamLink;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;
    /// Manning N
    float *m_ChManningN;

    //input from modules
    /// water depth for channel cell [mm], i.e., "HCH" from channel routing module
    float **m_ChannelWH;
    /// sediment flow into the channel [kg]
    float *m_SedToChannel;

    /// channel flow of each cell, [m3/s]
    float **m_ChQkin;
    /*/// overland flow for last time step, [m3/s]
    float** m_Qlastt;*/
    /// water volume in cell m3
    float **m_ChVol;
    /// flow velocity
    float **m_ChV;

    //output
    //// id of the outlet
    //int m_idOutlet;
    /// channel flow detachment [kg]
    float **m_CHDETFlow;
    /// sediment deposition[kg]
    float **m_CHSedDep;
    /// sediment concentration in flow [kg/m^3]
    float **m_CHSedConc;

    /// sediment content in flow [kg]
    float **m_CHSed_kg;
    /// outgoing sediment flux of the cell (kg/s), first is channel Id
    float **m_Qsn;
    /// sediment flux at subbasin (kg/s), useless? by LJ
    //float* m_SedSubbasin;

    //output for test
    float *m_detCH;
    float *m_depCh;
    float *m_routQs;
    float *m_cap;

    float *m_chanV;
    float *m_chanVol;


private:

    void initial();
    ///**
    //*	@brief calculate the velocity of overland flow.
    //*
    //*	@param R The hydraulic radius
    //*   @param S The sine of the Slope
    //*	@param n The Manning's n
    //*	@return the velocity V, m/s
    //*/
    //float CalcuVelocityChannelFlow(float R, float S, float n);
    //float CalcuVelocityChannelFlow(int ID);
    /**
    *	@brief calculate the channel flow velocity.  m/s
    *
    *	@param iReach The index in the array of m_reaches
    *
    *	@param iCell The index of cell array in each reach
    *
    *	@param ID The id of cell in grid map
    */
    void CalcuVelocityChannelFlow(int iReach, int iCell, int id);    // not used by here
    /**
    *	@brief calculate the flow detachment.
    *
    *	@param iReach The index in the array of m_reaches
    *
    *	@param iCell The index of cell array in each reach
    *
    *	@param ID The id of cell in grid map
    */
    void CalcuChFlowDetachment(int iReach, int iCell, int id);

    float GetTransportCapacity(int iReach, int iCell, int id);

    void GetSedimentInFlow(int iReach, int iCell, int id);

    /**
    *	@brief calculate the sediment routing of overland flow.
    *
    *	@param iReach The index in the array of m_reaches
    *
    *	@param iCell The index of cell array in each reach
    *
    *	@param ID The id of cell in grid map
    */
    void ChannelflowSedRouting(int iReach, int iCell, int id);

    //float MaxConcentration(float watvol, float sedvol);
    //float MaxConcentration(float watvol, float sedvol, int iReach, int iCell);

    /**
    *	@brief Simple calculation of sediment outflux from a cell based on the sediment concentration multiplied by the new water flux,
    *	j = time and i = place: j1i1 is the new output, j1i is the new flux at the upstream 'entrance' flowing into the gridcell
    *	@param  Qn  result kin wave for this cell
    *	@param  Qin  sum of all upstreamwater from kin wave
    *	@param  Sin  sum of all upstream sediment   kg/s
    *	@param  dt    the time step
    *	@param  vol   current volume of water in cell
    *	@param  sed    current mass of sediment in cell
    *	@return the newer sediment outflux, kg/s
    */
    float simpleSedCalc(float Qj1i1, float Qj1i, float Sj1i, float dt, float vol, float sed);

    /**
*	@briefComplex calculation of sediment outflux from a cell based on a explicit solution of the time/space matrix,
*     j = time and i = place: j1i1 is the new output, j1i is the new flux at the upstream 'entrance' flowing into the gridcell
*	@param Qj1i1   Qj+1,i+1 : result kin wave for this cell ;j = time, i = place
*	@param Qj1i    Qj+1,i   : sum of all upstreamwater from kin wave
*	@param Qji1    Qj,i+1 : incoming Q for kinematic wave (t=j) in this cell, map Qin in LISEM
*	@param Sj1i    Sj+1,i : sum of all upstream sediment
*	@param Sji1    Si,j+1 : incoming Sed for kinematic wave (t=j) in this cell, map Qsin in LISEM
*	@param alpha   alpha calculated in LISEM from before kinematic wave
*	@param dt      timestep
*	@param dx      dx: length of the cell, corrected for slope (DX map in LISEM)
*/
    float complexSedCalc(float Qj1i1, float Qj1i, float Qji1, float Sj1i, float Sji1, float alpha, float dt, float dx);


    void WaterVolumeCalc(int iReach, int iCell, int id);  //m3

    //set the input data which was not available right now, this will be delete when the data or module is available
    //void setNotAvailableInput();

};

#endif
