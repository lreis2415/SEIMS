/*!
 * \file StormGreenAmpt.h
 * \brief Green Ampt Method to calculate infiltration and excess precipitation
 * \author Junzhi Liu
 * \date Oct. 2011
 * 
 */
///USELESS? LJ
//#ifndef SEIMS_STORM_GA_INCLUDE
//#define SEIMS_STORM_GA_INCLUDE
#pragma once

#include <string>
#include "SimulationModule.h"

using namespace std;
/** \defgroup SUR_SGA
 * \ingroup Hydrology
 * \brief  Green Ampt Method to calculate infiltration and excess precipitation
 */
/*!
 * \class StormGreenAmpt
 * \ingroup SUR_SGA
 *
 * \brief Green Ampt Method to calculate infiltration and excess precipitation
 *
 */
class StormGreenAmpt : public SimulationModule
{
public:
    //! Constructor
    StormGreenAmpt(void);

    //! Destructor
    ~StormGreenAmpt(void);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void SetValue(const char *key, float value);

    virtual int Execute(void);

private:
    /**
    *	@brief check the input data. Make sure all the input data is available.
    *
    *	@return bool The validity of the input data.
    */
    bool CheckInputData(void);

    /**
    *	@brief check the input size. Make sure all the input data have same dimension.
    *
    *	@param key: The key of the input data
    *	@param n: The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int n);

    ///**
    //*	@brief check the output data. Make sure all the output data is available.
    //*
    //*	@param output1: the output variable PE
    //*	@param output2: the next output variable infiltration
    //*	@return bool The validity of the output data.
    //*/
    //bool CheckOutputData(float* output1, float* output2);

    void clearInputs(void);

    string getDate(time_t *date);

private:

    /// time step(seconds)
    float m_dt;

    /// count of valid cells
    int m_nCells;
    /// precipitation of each cell
    float *m_pNet;

    /// soil porosity
    float *m_porosity;
    /// soil moisture
    float *m_soilMoisture;
    /// root depth
    float *m_rootDepth;

    /// depression storage -- SD(t-1) from the depression storage module
    float *m_sd;

    /// temperature
    float *m_tMax, *m_tMin;
    /// snow fall temperature
    float m_tSnow;
    /// snow melt threshold temperature
    float m_t0;
    /// snow melt from the snow melt module  (mm)
    float *m_snowMelt;
    /// snow accumulation from the snow balance module (mm) at t+1 timestep
    float *m_snowAccu;

    /// threshold soil freezing temperature (��)
    float m_tSoilFrozen;
    /// frozen soil moisture relative to saturation above which no infiltration occur (m3/m3)
    float m_sFrozen;
    /// soil temperature obtained from the soil temperature module (��)
    float *m_soilTemp;

    /// parameters used in GA method
    /// saturated hydraulic conductivity from parameter database (m/s)
    float *m_ks;
    /// percent of clay content from parameter database
    float *m_clay;
    /// percent of sand content from parameter database
    float *m_sand;
    /// initial soil moisture
    float *m_initSoilMoisture;
    /// field capacity
    float *m_fieldCap;
    /// surface water depth
    float *m_sr;

    /// intermediate variables
    /// Soil Capillary Suction Head (m)
    float *m_capillarySuction;
    /// cumulative infiltration depth (m)
    float *m_accumuDepth;


    // output
    /// infiltration map of watershed (mm) of the total nCells
    float *m_infil;
    /**
    *	@brief surplus of filtration capacity.
    *
    * if pNet > infilPotential, surplus = 0
    * else surplus = infilPotential - pNet
    */
    float *m_infilCapacitySurplus;

    /// this function calculated the wetting front matric potential
    float CalculateCapillarySuction(float por, float clay, float sand);

    void initialOutputs();

};
///#endif
