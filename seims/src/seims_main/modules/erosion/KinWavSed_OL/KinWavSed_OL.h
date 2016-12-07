/*!
 * \file KinWavSed_OL.h
 * \brief Kinematic wave routing method for overland erosion and deposition
 * \author Hui Wu
 * \date Feb. 2012
 * \revised LiangJun Zhu
 * \revised date May. 2016
 */

#ifndef SEIMS_KinWavSed_OL_INCLUDE
#define SEIMS_KinWavSed_OL_INCLUDE

#include <string>
#include <ctime>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup KinWavSed_OL
 * \ingroup Erosion
 * \brief Kinematic wave method for overland flow erosion and deposition
 */
/*!
 * \class KinWavSed_OL
 * \ingroup KinWavSed_OL
 *
 * \brief Kinematic wave method for overland flow erosion and deposition
 *
 */
class KinWavSed_OL : public SimulationModule
{
public:
    KinWavSed_OL(void);

    ~KinWavSed_OL(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

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

    /// static string toString(float value); replaced by ValueToString() defined in util module. by LJ

    void initial();

    void GetTransportCapacity(int ID);

    void GetSedimentInFlow(int ID);

    /**
    *	@brief calculate the velocity of overland flow.
    */
    void CalcuVelocityOverlandFlow();

    /**
    *	@brief calculate the velocity of overland flow.
    */
    void CalcuVelocityChannelFlow();

    /**
    *	@brief calculate the flow detachment.
    *
    *	@param ID The id of cell in grid map
    */
    void CalcuFlowDetachment(int ID);

    /**
    *	@brief calculate the sediment routing of overland flow.
    *
    *	@param ID The id of cell in grid map
    */
    void OverlandflowSedRouting(int ID);

    ///**
    //*	@brief calculate the sediment routing of overland flow.
    //*
    //*	@param ID The id of cell in grid map
    //*	@return the sediment of flowing to channel for each channel cell, kg
    //*/
    float SedToChannel(int ID);

    void MaxConcentration(float watvol, float sedvol, int id);

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

    //float WaterVolumeCalc(int id);  //m3
    void WaterVolumeCalc();

    //set the input data which was not available right now, this will be delete when the data or module is available
    //void setNotAvailableInput();

private:
    //Parameters
    ///calibration coefficient of overland erosion (-)
    float m_Ccoe;
    /// calibration coefficient 1
    float m_eco1;
    /// calibration coefficient 2
    float m_eco2;

    /**
    *	@brief Routing layers according to the flow direction
    *
    *	There are not flow relationships within each layer.
    *	The first element in each layer is the number of cells in the layer
    */
    float **m_routingLayers;
    /**
    *	@brief 2d array of flow in cells
    *
    *	The first element in each sub-array is the number of flow in cells in this sub-array
    */
    float **m_flowInIndex;

    int m_nLayers;

    /// cell width of grid map (m)
    float m_CellWidth;
    /// number of cells
    int m_nCells;
    /// length of time step (s)
    float m_TimeStep;
    /// slope of map, to calculate slope gradient.
    float *m_Slope;
    /// crop management factor
    float *m_USLE_C;
    /// soil erodibility factor
    float *m_USLE_K;

    //input from modules
    /// Runoff mm
    //float* m_Runoff; // I think it is the same as m_WH
    /// water depth available for surface water [mm],
    float *m_WH;
    ///the distribution of splash detachment,(kg)
    float *m_DETSplash;
    /// flow width (m) of overland plane
    float *m_FlowWidth;
    /// channel width (zero for non-channel cells)
    float *m_chWidth;
    /// kinematic wave flow in the time step, m3/s
    float *m_Qkin;
    /// Water depth added to channel water depth
    float *m_whtoCh;
    /// flow velocity
    float *m_V;
    /// flow volum in cell
    float *m_Vol;
    /// Manning's roughness [-]
    float *m_ManningN;
    /// streamlink
    float *m_streamLink;

    //temporal variables
    /// Soil transport capacity of overland flow (kg) at each time step
    float *m_Ctrans;
    /// sediment content in flow [kg]
    float *m_Sed_kg;
    /// outgoing sediment flux (kg/s)
    float *m_Qsn;

    //output
    /// the distribution of overland flow detachment, kg/cell
    float *m_DETOverland;
    /// sediment deposition[kg]
    float *m_SedDep;
    /*/// sediment concentration in flow [kg/m^3]
    float* m_SedConc;*/
    /// sediment flow into the channel [kg]
    float *m_SedToChannel;

    //test
    float *m_ChV;
    float *m_QV;
    float *m_fract;

};

#endif
