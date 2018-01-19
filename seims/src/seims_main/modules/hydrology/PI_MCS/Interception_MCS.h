/*!
 * \brief Precipitation interception based on Maximum Canopy Storage
 *        Equation given by Aston(1979), Code adapted from openLISEM
 *        file: lisInterception.cpp
 *        This module is STORM_MODE and LONGTERM_MODE compatibility.
 * \author Liangjun Zhu
 * \date Apr 2017
 * 
 */
#pragma once
#include "SimulationModule.h"

using namespace std;
/** \defgroup PI_MCS
 * \ingroup Hydrology
 * \brief Module for Precipitation Interception module based on Maximum Canopy Storage.
 */

/*!
 * \class clsPI_MCS
 * \ingroup PI_MCS
 * \brief Class for Precipitation Interception module based on Maximum Canopy Storage.
 */
class clsPI_MCS : public SimulationModule {
public:
    //! Constructor
    clsPI_MCS(void);

    //! Destructor
    ~clsPI_MCS(void);
private:
    /* Parameters from database */

    // The embankment area ratio of paddy rice HRU
    float m_embnkfr_pr;
    // The fraction of precipitation fall on the embankment that drain into ditches or canals directly
    float m_pcp2canfr_pr;
    // landuse
    float *m_landuse;
    //! Calibration parameter, the sine-shaped curve controller exponent b, default is 1.35
    float m_Pi_b;
    //! Calibration parameter, the initial interception storage for all cells, mm
    float m_Init_IS;
    //! Maximum storage capacity, mm
    float *m_maxSt;
    //! Minimum storage capacity, mm
    float *m_minSt;
#ifdef STORM_MODE
    //! hillslope time step, seconds
    float m_hilldt;
    //! slope for rainfall correction, height/width, i.e. tan(slope)
    float *m_slope;
#endif

    /* Input variables from other module's output */

    /*! Precipitation
     * For STROM_MODE model, the unit is rainfall intensity mm/h
     * For LONGTERM_MODE model, the unit is mm
     */
    float *m_P;
#ifndef STORM_MODE
    //! PET, mm
    float *m_PET;
#endif

    /* Results */

    //! current interception storage, the initial value equal to 0, mm
    float *m_st;
    //! Interception loss, mm
    float *m_interceptionLoss;
#ifndef STORM_MODE
    //! Evaporation loss from intercepted rainfall, mm
    float *m_evaporationLoss;
#endif
    //! Net precipitation (after slope correction, of course), mm
    float *m_netPrecipitation;

    /* Others */

    //!  number of valid cells
    int m_nCells;
public:
    virtual void Set1DData(const char *key, int nRows, float *data);

    virtual void SetValue(const char *key, float data);

    virtual void Get1DData(const char *key, int *nRows, float **data);

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
    *	@param key The key of the input data
    *	@param n The input data dimension
    *	@return bool The validity of the dimension
    */
    bool CheckInputSize(const char *, int);

    /*!
     * \brief Initialize output variables for the first run of the entire simulation
     */
    void initialOutputs(void);
};
