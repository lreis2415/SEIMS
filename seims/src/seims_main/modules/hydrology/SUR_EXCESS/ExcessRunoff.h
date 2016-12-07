/** 
*	@file
*	@version	1.0
*	@author    Junzhi Liu
*	@date	31-Octobor-2011
*
*	@brief	Green Ampt Method to calculate infiltration and excess precipitation
*
*/

#ifndef SEIMS_STORM_GA_INCLUDE
#define SEIMS_STORM_GA_INCLUDE

#include <string>
#include "SimulationModule.h"

using namespace std;

class ExcessRunoff : public SimulationModule
{
public:
    ExcessRunoff(void);

    ~ExcessRunoff(void);

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
    /// field capacity
    float *m_fieldCap;
    /// soil moisture
    float *m_soilMoisture;
    /// root depth
    float *m_rootDepth;

    /// depression storage -- SD(t-1) from the depression storage module
    float *m_sd;

    /// parameters used in GA method
    /// saturated hydraulic conductivity from parameter database (m/s)
    float *m_ks;
    /// initial soil moisture
    float *m_initSoilMoisture;

    /// cumulative infiltration depth (m)
    float *m_accumuDepth;


    // output
    /// the excess precipitation (mm) of the total nCells
    float *m_pe;
    /// infiltration map of watershed (mm) of the total nCells
    float *m_infil;
    /**
    *	@brief surplus of filtration capacity.
    *
    * if pNet > infilPotential, surplus = 0
    * else surplus = infilPotential - pNet
    */
    float *m_infilCapacitySurplus;

};

#endif
