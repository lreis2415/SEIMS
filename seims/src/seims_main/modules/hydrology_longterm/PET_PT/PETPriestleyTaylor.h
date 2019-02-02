/*!
 * \file PETPriestleyTaylor.h
 * \brief Potential evapotranspiration using PriestleyTaylor method
 *
 * Changelog:
 *   - 1. 2010-11-30 - jz - Initial implementation.
 *   - 2. 2016-05-30 - lj -
 *        -# Add m_tMean from database, which may be measurement value or the mean of tMax and tMin
 *        -# The PET calculate is changed from site-based to cell-based, because PET is not only dependent on Climate site data
 *        -# Add m_VPD, m_dayLen as outputs, which will be used in PG_EPIC module
 *        -# Add m_phuBase as outputs, which will be used in MGT_SWAT module
 *
 * \author Junzhi Liu, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_PET_PT_H
#define SEIMS_MODULE_PET_PT_H

#include "SimulationModule.h"

/*!
 * \defgroup PET_PT
 * \ingroup Hydrology_longterm
 * \brief Calculate potential evapotranspiration using PriestleyTaylor method
 *
 */
/*!
 * \class PETPriestleyTaylor
 * \ingroup PET_PT
 *
 * \brief Priestley Taylor Method to Compute PET
 *
 */
class PETPriestleyTaylor: public SimulationModule {
public:
    PETPriestleyTaylor();

    ~PETPriestleyTaylor();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* value) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    /// mean air temperature for a given day(degree)
    float* m_meanTemp;
    /// maximum air temperature for a given day(degree)
    float* m_maxTemp;
    /// minimum air temperature for a given day(degree)
    float* m_minTemp;
    /// solar radiation(MJ/m2/d)
    float* m_sr;
    /// relative humidity(%)
    float* m_rhd;
    /// elevation(m)
    float* m_dem;
    /// valid cells number
    int m_nCells;
    /// Correction Factor for PET
    float m_petFactor;
    ///latitude of the stations
    float* m_cellLat;
    /// annual PHU
    float* m_phuAnn;
    ///The temperature of snow melt
    float m_snowTemp;

    /// output

    /// day length (hr)
    float* m_dayLen;
    /// base zero total heat units (used when no land cover is growing)
    float* m_phuBase;
    /// pet
    float* m_pet;
    /// vapor pressure deficit
    float* m_vpd;
};
#endif /* SEIMS_MODULE_PET_PT_H */
