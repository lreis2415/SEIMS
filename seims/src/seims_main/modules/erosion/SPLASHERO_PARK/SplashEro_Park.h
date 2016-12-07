/*!
 * \file SplashEro_Park.h
 * \brief Park Equation for splash erosion, and Foster Equation for overland flow soil detachment
 *           use the USLE_C, USLE_K in the calculation of splash erosion.
 * \author Hui Wu
 * \date Feb. 2012
 */
#pragma once
#ifndef SEIMS_SplashEro_Park_INCLUDE
#define SEIMS_SplashEro_Park_INCLUDE

#include <string>
#include <ctime>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup SplashEro_Park
 * \ingroup Erosion
 * \brief Park Equation for splash erosion
 */
/*!
 * \class SplashEro_Park
 * \ingroup SplashEro_Park
 *
 * \brief Park Equation for splash erosion, and Foster Equation for overland flow soil detachment
 *           use the USLE_C, USLE_K in the calculation of splash erosion.//
 *
 */
class SplashEro_Park : public SimulationModule
{
public:
    //! Constructor
    SplashEro_Park(void);

    //! Destructor
    ~SplashEro_Park(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

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

    //static string toString(float value);

    void initialOutputs();

private:
    //Parameters

    /// cell width of grid map (m)
    float m_CellWith;
    /// number of cells
    int m_nCells;
    /// length of time step (s)
    float m_TimeStep;

    ///parameter calibration coefficient of splash erosion (-)
    float m_Omega;
    ///parameter calibration coefficient of splash erosion (-)  /// NOT USED, deleted? LJ
    float m_Ccoe;
    /// slope of map, to calculate slope gradient.
    float *m_Slope;
    /// fraction of stones on the surface, affects splash [-]
    //float* m_GrassFrac;
    /// fraction of vegetation canopy cover [-]
    //float* m_coverFrac;
    /// channel width [m]
    //float* m_ChWidth;
    /// crop management factor
    float *m_USLE_C;
    /// soil erodibility factor
    float *m_USLE_K;

    //input from modules
    /// the depth of the surface water layer (mm), after kinematic wave model
    /// WaterDepth = Depression + SurfaceRunoffDepth
    float *m_sr;
    float *m_depression;

    /// water flux after kinematic wave model, m3/s
    float *m_Q;
    /// the amount of rainfall (mm) (pNet?)
    float *m_Rain;
    /// snowmelt cover map, value 1.0 if there is snowcover, 0 without [-], from distribution of snow accumulation from snow water balance module
    //float* m_SnowCover;

    //output
    /// the distribution of splash detachment, kg/cell
    float *m_DETSplash;

};

#endif
