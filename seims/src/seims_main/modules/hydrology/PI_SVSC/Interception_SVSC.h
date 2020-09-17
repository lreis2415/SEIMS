/*!
 * \brief Precipitation interception by Seasonal Variation of Storage Capacity method.
 *        Algorithm adapted from WetSpa, refers to WetSpa Manual p.24-26.
 *        This module is STORM_MODE and LONGTERM_MODE compatibility.
 * \author Alex Storey, Junzhi Liu, Liangjun Zhu
 * \date Apr 2017
 *
 */
#ifndef SEIMS_PI_SVSC_H
#define SEIMS_PI_SVSC_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/** \defgroup PI_SVSC
 * \ingroup Hydrology
 * \brief Module for Precipitation Interception module by Seasonal Variation of Storage Capacity method.
 */

/*!
 * \class clsPI_SVSC
 * \ingroup PI_SVSC
 * \brief Class for Precipitation Interception module by Seasonal Variation of Storage Capacity method.
 *        1. Lookup for max. and min. interception storage capacity corresponding to summer and winter
 *           extremes for different vegetation types.
 *        2. A simple sine-shaped variation curve is used to interpolate for continuous storage capacity.
 *        3. Hourly interception storage capacity is assumed to be constant.
 *        4. Evaporation is only accounted for LONGTERM_MODE
 */
class clsPI_SVSC : public SimulationModule {
public:
    //! Constructor
    clsPI_SVSC(void);

    //! Destructor
    ~clsPI_SVSC(void);
private:
    /* Parameters from database */

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
    void InitialOutputs(void);
};
#endif /* SEIMS_PI_SVSC_H */
