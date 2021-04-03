/*!
 * \file AET_PriestleyTaylorHargreaves.h
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 * and potential and actual soil evaporation.
 *
 * Code from etact.f of SWAT source.
 *
 * Changelog:
 *   - 1. 2016-07-15 - lj -
 *      -# Code reformat with common functions, such as Release1DArray.
 *      -# VAR_SNSB[0] should be output other than input.
 *   - 2. 2018-05-07 - lj - Reformat code style.
 *   - 3. 2018-06-26 - lj - Bug fixed when pet less than intercept ET.
 *
 * \author Liang-Jun Zhu
 * \date May 2016
 */
#ifndef SEIMS_MODULE_AET_PTH_H
#define SEIMS_MODULE_AET_PTH_H

#include "SimulationModule.h"

/** \defgroup AET_PTH
 * \ingroup Ecology
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 *Actual soil evaporation is also calculated.
 */
/*!
 * \class AET_PT_H
 * \ingroup AET_PTH
 *
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 * Actual soil evaporation is also calculated.
 */
class AET_PT_H: public SimulationModule {
public:
    AET_PT_H();

    ~AET_PT_H();

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    // Parameters from database
    int m_nCells;      ///< valid cells number
    int m_maxSoilLyrs; ///< maximum soil layers, mlyr in SWAT

    float* m_esco;       ///< soil evaporation compensation factor, 0.01-1.0, default of 0.9
    float* m_nSoilLyrs;  ///< soil layers
    float** m_soilDepth; ///< soil depth
    float** m_soilThk;   ///< soil thickness
    float** m_solFC;     ///< amount of water available to plants in soil layer at field capacity (FC-WP)
    float* m_rsdCovSoil; ///< amount of residue on soil surface (kg/ha)
    float** m_solNo3;    ///< amount of nitrogen stored in the nitrate pool

    // Inputs from other modules
    float* m_tMean;          ///< mean air temperature (deg C)
    float* m_lai;            ///< leaf area index(m^2/m^2)
    float* m_pet;            ///< potential evapotranspiration
    float* m_IntcpET;        ///< Evaporation loss from canopy storage
    float* m_snowAccum;      ///< amount of water in snow
    float* m_snowSublim;     ///< snow sublimation
    float** m_soilWtrSto;    ///< soil storage of each soil layer (mm), sol_st in SWAT
    float* m_soilWtrStoPrfl; ///< soil water storage in soil profile (mm), sol_sw in SWAT

    // Outputs
    float* m_maxPltET; ///< maximum amount of transpiration (plant et), ep_max in SWAT
    float* m_soilET;   ///< actual amount of evaporation (soil et), es_day in SWAT
};
#endif /* SEIMS_MODULE_AET_PTH_H */
