/*!
 * \file SoilTemperatureFINPL.h
 * \brief Finn Plauborg Method to Compute Soil Temperature
 *
 * Changelog:
 *   - 1. 2011-01-05 - jz - Initial implementation.
 *   - 2. 2016-05-27 - lj - Code review and reformat.
 *
 * \author Junzhi Liu, Liangjun Zhu
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

const float radWt = 0.01721420632103996f; /// PI * 2.f / 365.f;

/*!
 * \class SoilTemperatureFINPL
 * \ingroup STP_FP
 * \brief Soil temperature
 *
 */
class SoilTemperatureFINPL: public SimulationModule {
public:
    SoilTemperatureFINPL();

    ~SoilTemperatureFINPL();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    /// from parameter database
    /// coefficients in the Equation
    float m_a0, m_a1, m_a2, m_a3, m_b1, m_b2, m_d1, m_d2;
    /// ratio between soil temperature at 10 cm and the mean
    float m_kSoil10;

    /// count of cells
    int m_nCells;
    /// factor of soil temperature relative to short grass (degree)
    float* m_soilTempRelFactor10;
    /// landuse type, for distinguish calculation, such as water body.
    float* m_landUse;
    /// from interpolation module
    /// mean air temperature of the current day
    float* m_meanTemp;
    ///// mean air temperature of the day(d-1)
    float* m_meanTempPre1;
    ///// mean air temperature of the day(d-2)
    float* m_meanTempPre2;

    /// output soil temperature
    float* m_soilTemp;
};
#endif /* SEIMS_MODULE_STP_FP_H */
