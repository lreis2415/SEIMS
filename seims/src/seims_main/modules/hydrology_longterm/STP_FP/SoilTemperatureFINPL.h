/**
* @brief	Finn Plauborg Method to Compute Soil Temperature
* @author	Junzhi Liu
* @date	5-January-2011
* @revised LiangJun Zhu
* @date 27-May-2016
*
*/
#ifndef SEIMS_MODULE_STP_FP_H
#define SEIMS_MODULE_STP_FP_H

#include "SimulationModule.h"

/*!
 * \defgroup STP_FP
 * \ingroup Hydrology_longterm
 * \brief Finn Plauborg Method to Compute Soil Temperature
 *
 */

/*!
 * \class SoilTemperatureFINPL
 * \ingroup STP_FP
 * \brief Soil temperature
 *
 */
class SoilTemperatureFINPL : public SimulationModule {
public:
    SoilTemperatureFINPL();

    ~SoilTemperatureFINPL();

    void SetValue(const char *key, float value) override;

    void Set1DData(const char *key, int n, float *data) override;

    void Get1DData(const char *key, int *n, float **data) override;

    int Execute() override;

private:
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
    bool CheckInputSize(const char *key, int n);

    /*!
    * \brief Initialize output variables for the first run of the entire simulation
    */
    void  InitialOutputs();

private:
    /// from parameter database
    /// coefficients in the Equation
    float m_a0, m_a1, m_a2, m_a3, m_b1, m_b2, m_d1, m_d2;
    /// ratio between soil temperature at 10 cm and the mean
    float m_kSoil10;

    /// Julian day
    int m_julianDay;
    /// count of cells
    int m_nCells;
    /// factor of soil temperature relative to short grass (degree)
    float *m_relativeFactor;
    /// landuse type, for distinguish calculation, such as water body.
    float *m_landuse;
    /// from interpolation module
    /// mean air temperature of the current day
    float *m_tMean;
    ///// mean air temperature of the day(d-1)
    float *m_t1;
    ///// mean air temperature of the day(d-2)
    float *m_t2;
    /// temporary variable
    float w;

    /// output soil temperature
    float *m_soilTemp;
};
#endif /* SEIMS_MODULE_STP_FP_H */
