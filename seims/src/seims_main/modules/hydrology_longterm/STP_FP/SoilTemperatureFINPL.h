/** 
* @file
* @version	1.1
* @author	Junzhi Liu
* @date	5-January-2011
* @revised LiangJun Zhu
* @date 27-May-2016
* @brief	Finn Plauborg Method to Compute Soil Temperature
*
*/
#ifndef SEIMS_STP_FP_INCLUDE
#define SEIMS_STP_FP_INCLUDE

#include <string>
#include "api.h"
#include "SimulationModule.h"

using namespace std;
/** \defgroup STP_FP
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
class SoilTemperatureFINPL : public SimulationModule
{
public:
    //! Constructor
    SoilTemperatureFINPL(void);

    //! Destructor
    ~SoilTemperatureFINPL(void);

    virtual void SetValue(const char *key, float value);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual int Execute();

    //virtual void SetDate(time_t t);

private:

    /// time
    //time_t m_date;

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

    /*!
     * \brief Initialize output variables for the first run of the entire simulation
     */
    void initialOutputs();

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
};

#endif