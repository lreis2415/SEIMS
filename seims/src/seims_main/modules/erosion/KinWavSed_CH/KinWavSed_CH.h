/*!
 * \brief Kinematic wave method for channel flow erosion and deposition
 * \author Hui Wu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */
#ifndef SEIMS_MODULE_KINWAVSED_CH_H
#define SEIMS_MODULE_KINWAVSED_CH_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/*! \defgroup KinWavSed_CH
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
class KinWavSed_CH : public SimulationModule {
public:
    //! Constructor
    KinWavSed_CH();

    //! Destructor
    ~KinWavSed_CH();

    //! Execute
    virtual int Execute();

    virtual void SetValue(const char *key, FLTPT value);

    virtual void SetValue(const char* key, int data);

    virtual void Set1DData(const char *key, int n, FLTPT *data);

    virtual void Get1DData(const char *key, int *n, FLTPT **data);

    virtual void Set1DData(const char* key, int n, int* data);

    virtual void Set2DData(const char *key, int nrows, int ncols, FLTPT **data);

    virtual void Set2DData(const char* key, int nrows, int ncols, int** data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, FLTPT ***data);

    virtual void SetReaches(clsReaches *reaches);

    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData();

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
    /**
    *	@brief reach layers map
    *
    *	The elements are the index of reach array
    */
    map<int, vector<int> > m_reachLayers;
    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    int **m_flowInIndex;
    /// flow out index
    int **m_flowOutIdx;
    /// channel width (zero for non-channel cells)
    FLTPT *m_chWidth;
    /// stream order
    int *m_streamOrder;
    /// stream id of downstream
    FLTPT *m_reachDownStream;
    /// Index of upstream Ids (The value is -1 if there if no upstream reach)
    vector<vector<int> > m_reachUpStream;
    /**
    *	@brief reach links
    *
    *	key: index of the reach
    *	value: vector of cell index
    */
    map<int, vector<int> > m_reachs;

    /// cell width of grid map (m)
    FLTPT m_CellWith;
    /// number of valid cells
    int m_nCells;
    /// length of time step (s)
    int m_TimeStep;
    /// layering method, 0 means UP_DOWN, 1 means DOWN_UP
    //LayeringMethod m_layeringMethod;
    ///calibration coefficient of transport capacity
    FLTPT m_ChTcCo;
    /// calibration coefficient of channel flow detachment
    FLTPT m_ChDetCo;
    /// slope of map, to calculate slope gradient.
    FLTPT*m_Slope;
    /// USLE K
    FLTPT**m_USLE_K;
    /*/// hydraulic radius (m)
    float* m_R;*/
    /// id of source cells of reaches
    int *m_sourceCellIds;
    /// channel number
    int m_chNumber;
    /// stream link
    int *m_streamLink;
    /// map from subbasin id to index of the array
    map<int, int> m_idToIndex;
    /// Manning N
    FLTPT *m_ChManningN;

    //input from modules
    /// water depth for channel cell [mm], i.e., "HCH" from channel routing module
    FLTPT **m_ChannelWH;
    /// sediment flow into the channel [kg]
    FLTPT *m_SedToChannel;

    /// channel flow of each cell, [m3/s]
    FLTPT **m_ChQkin;
    /*/// overland flow for last time step, [m3/s]
    FLTPT** m_Qlastt;*/
    /// water volume in cell m3
    FLTPT **m_ChVol;
    /// flow velocity
    FLTPT **m_ChV;

    //output
    //// id of the outlet
    //int m_idOutlet;
    /// channel flow detachment [kg]
    FLTPT **m_CHDETFlow;
    /// sediment deposition[kg]
    FLTPT **m_CHSedDep;
    /// sediment concentration in flow [kg/m^3]
    FLTPT **m_CHSedConc;

    /// sediment content in flow [kg]
    FLTPT **m_CHSed_kg;
    /// outgoing sediment flux of the cell (kg/s), first is channel Id
    FLTPT **m_Qsn;
    /// sediment flux at subbasin (kg/s), useless? by LJ
    //FLTPT* m_SedSubbasin;

    //output for test
    FLTPT *m_detCH;
    FLTPT *m_depCh;
    FLTPT *m_routQs;
    FLTPT *m_cap;

    FLTPT *m_chanV;
    FLTPT *m_chanVol;

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

    FLTPT GetTransportCapacity(int iReach, int iCell, int id);

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
    FLTPT simpleSedCalc(FLTPT Qj1i1, FLTPT Qj1i, FLTPT Sj1i, FLTPT dt, FLTPT vol, FLTPT sed);

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
    FLTPT complexSedCalc(FLTPT Qj1i1, FLTPT Qj1i, FLTPT Qji1, FLTPT Sj1i, FLTPT Sji1, FLTPT alpha, FLTPT dt, FLTPT dx);

    void WaterVolumeCalc(int iReach, int iCell, int id);  //m3

    //set the input data which was not available right now, this will be delete when the data or module is available
    //void setNotAvailableInput();

};

#endif /* SEIMS_MODULE_KINWAVSED_CH_H */
