/*!
 * \file Interception_MCS.h
 * \brief Precipitation interception based on Maximum Canopy Storage
 *        Equation given by Aston(1979), Code adapted from openLISEM
 *        file: lisInterception.cpp
 *
 *        This module is STORM_MODE and LONGTERM_MODE compatibility.
 *
 * \author Liangjun Zhu
 * \date Apr 2017
 */
#ifndef SEIMS_MODULE_PI_MCS_H
#define SEIMS_MODULE_PI_MCS_H

#include "SimulationModule.h"

/** \defgroup PI_MCS
 * \ingroup Hydrology
 * \brief Module for Precipitation Interception module based on Maximum Canopy Storage.
 */

/*!
 * \class clsPI_MCS
 * \ingroup PI_MCS
 * \brief Class for Precipitation Interception module based on Maximum Canopy Storage.
 */
class clsPI_MCS: public SimulationModule {
public:
    clsPI_MCS();

    ~clsPI_MCS();

    void Set1DData(const char* key, int nRows, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int nRows, int* data) OVERRIDE;

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void SetValue(const char* key, int value) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* nrows, FLTPT** data) OVERRIDE;

private:
    /* Parameters from database */

    // The embankment area ratio of paddy rice cells
    FLTPT m_embnkFr;
    // The fraction of precipitation fall on the embankment that drain into ditches or canals directly
    FLTPT m_pcp2CanalFr;
    // landuse
    int* m_landUse;
    //! Calibration parameter of interception storage, the sine-shaped curve controller exponent b, default is 1.35
    FLTPT m_intcpStoCapExp;
    //! Calibration parameter, the initial interception storage for all cells, mm
    FLTPT m_initIntcpSto;
    //! Maximum storage capacity, mm
    FLTPT* m_maxIntcpStoCap;
    //! Minimum storage capacity, mm
    FLTPT* m_minIntcpStoCap;
#ifdef STORM_MODE
    //! hillslope time step, seconds
    int m_hilldt;
    //! slope for rainfall correction, height/width, i.e. tan(slope)
    FLTPT* m_slope;
#endif

    /* Input variables from other module's output */

    /*! Precipitation
     * For STROM_MODE model, the unit is rainfall intensity mm/h
     * For LONGTERM_MODE model, the unit is mm
     */
    FLTPT* m_pcp;
#ifndef STORM_MODE
    //! PET, mm
    FLTPT* m_pet;
#endif

    /* Results */

    //! current interception storage, the initial value equal to 0, mm
    FLTPT* m_canSto;
    //! Interception loss, mm
    FLTPT* m_intcpLoss;
#ifndef STORM_MODE
    //! Evaporation loss from intercepted rainfall, mm
    FLTPT* m_IntcpET;
#endif
    //! Net precipitation (after slope correction, of course), mm
    FLTPT* m_netPcp;

    /* Others */

    //!  number of valid cells
    int m_nCells;
};
#endif /* SEIMS_MODULE_PI_MCS_H */
