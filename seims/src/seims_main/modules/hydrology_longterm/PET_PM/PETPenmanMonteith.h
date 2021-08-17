/*!
 * \file PETPenmanMonteith.h
 * \brief Potential evapotranspiration using Penman Monteith Method.
 *
 * Changelog:
 *   - 1. 2010-11-30 - jz - Initial implementation.
 *   - 2. 2016-05-30 - lj -
 *        -# Add m_tMean from database, which may be measurement value or the mean of tMax and tMin
 *        -# The PET calculate is changed from site-based to cell-based, because PET is not only dependent on Climate site data
 *        -# Add ecology related parameters (initialized value)
 *        -# Add potential plant transpiration as output
 *        -# Add m_VPD, m_dayLen as outputs, which will be used in PG_EPIC module
 *        -# change m_vpd2 and m_gsi from DT_Single to DT_Raster1D, see readplant.f of SWAT
 *        -# Add m_phuBase as outputs, which will be used in MGT_SWAT module
 *
 * \author Junzhi Liu, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_PET_PM_H
#define SEIMS_MODULE_PET_PM_H

#include "SimulationModule.h"

/*!
 * \defgroup PET_PM
 * \ingroup Hydrology_longterm
 * \brief Penman Monteith Method to Compute PET
 *
 */

/*!
 * \class PETPenmanMonteith
 * \ingroup PET_PM
 *
 * \brief Penman Monteith Method to Compute PET
 *
 */
class PETPenmanMonteith: public SimulationModule {
public:
    PETPenmanMonteith();

    ~PETPenmanMonteith();

    void Set1DData(const char* key, int n, float* value) OVERRIDE;

    void SetValue(const char* key, float value) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void InitializeIntermediateVariables() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    /// Mean air temperature for a given day (deg C)
    float* m_meanTemp;
    /// Minimum air temperature for a given day (deg C)
    float* m_minTemp;
    /// Maximum air temperature for a given day (deg C)
    float* m_maxTemp;
    /// solar radiation(MJ/m2/d)
    float* m_sr;
    /// relative humidity(%)
    float* m_rhd;
    /// wind speed
    float* m_ws;
    /// elevation(m)
    float* m_dem;
    /// annual PHU
    float* m_phuAnn;
    /**
    * @brief land cover status code
    *
    * 0 no land cover currently growing
    * 1 land cover growing
    */
    float* m_igro;
    /// canopy height for the day(m)
    float* m_canHgt;
    /// leaf area index(m2/m2)
    float* m_lai;
    /// albedo in the day
    float* m_alb;
    /// valid cells number
    int m_nCells;
    ///latitude of each valid cells
    float* m_cellLat;

    /// CO2 concentration(ppmv)
    float m_co2Conc;
    /// rate of decline in stomatal conductance per unit increase in vapor pressure deficit(m/s/kPa)
    float* m_vpd2;
    /// maximum stomatal conductance(m/s) at high solar radiation and low vpd
    float* m_gsi;
    /// vapor pressure deficit(kPa) corresponding to the second point on the stomatal conductance curve
    float* m_vpdfr;
    /// fraction of maximum stomatal conductance corresponding to the second point on the stomatal conductance curve
    float* m_frgmax;
    ///The temperature of snow melt
    float m_snowTemp;
    /// Correction Factor for PET
    float m_petFactor;
    /// output pet array
    float* m_pet;
    /// maximum amount of transpiration (plant et)  that can occur on current day in HRU
    float* m_maxPltET;
    /// vapor pressure deficit
    float* m_vpd;
    /// day length
    float* m_dayLen;
    /// base zero total heat units (used when no land cover is growing)
    float* m_phuBase;
};
#endif /* SEIMS_MODULE_PET_PM_H */
