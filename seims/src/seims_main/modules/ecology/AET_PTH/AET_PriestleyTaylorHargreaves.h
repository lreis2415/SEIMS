/*!
 * \brief Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods
 * and potential and actual soil evaporation.
 * Code from SWAT, etact.f
 * \author Liang-Jun Zhu
 * \date May 2016
 * \changelog: 2016-07-15 - lj - 1. Code reformat with common functions, such as Release1DArray.\n
 *                               2. VAR_SNSB should be output other than input.\n
 *             2018-05-07 - lj - Reformat code style.\n
 *             2018-06-26 - lj - Bug fixed when pet less than intercept ET.\n
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

    int Execute() OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

private:

    // compute soil evaporation and crop transpiration by SWAT method
    float AET_PT_H::SWAT_maxPET(float pet, int i);

    // compute soil evaporation and crop transpiration by ORYZA method
    float AET_PT_H::ORYZA_maxPET(float pet, int i);

    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData();

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char* key, int n);

    //! initialize outputs
    void InitialOutputs();
private:
    /// landuse
    float* m_landuse;
    /// Crop stage,0=before sowing; 1=sowing; 2=in seedbed; 3=day of transplanting; 4=main growth period, should be get value at PLTMGT_SWAT
    float* m_cropsta;
    /// impounding trigger
    float* m_impoundTrig;
    /// pothole volume, mm
    float* m_potVol;
    /// valid cells number
    int m_nCells;
    /// leaf area index(m^2/m^2)
    float* m_lai;
    /// potential evapotranspiration on current day
    float* m_pet;
    /// Evaporation loss from canopy storage
    float* m_IntcpET;
    /// depression storage capacity, not used!
    ///float* m_depSt;
    /// soil evaporation compensation factor, if not set or existed, it will be assigned 0.95 as default.
    /// esco should be vary from 0.01 to 1.0
    float* m_esco;
    /// soil layers
    float* m_nSoilLyrs;
    /// maximum soil layers, mlyr in SWAT
    int m_maxSoilLyrs;
    /// soil depth
    float** m_soilDepth;
    /// soil thickness
    float** m_soilThk;
    /// amount of water available to plants in soil layer at field capacity (FC-WP)
    float** m_solFC;
    /// amount of residue on soil surface (kg/ha)
    float* m_rsdCovSoil;
    /// amount of nitrogen stored in the nitrate pool
    float** m_solNo3;
    /// mean air temperature (deg C)
    float* m_tMean;
    /// amount of water in snow on current day
    float* m_snowAccum;
    /// snow sublimation on current day
    float* m_snowSublim;
    /// soil storage of each soil layer, mm H2O
    float** m_soilWtrSto;
    /// soil water storage in soil profile (mm)
    float* m_soilWtrStoPrfl;
    /// add output variables

    /// maximum amount of transpiration (plant et) that can occur on current day, ep_max in SWAT
    float* m_maxPltET;
    /// actual amount of evaporation (soil et) that occurs on day, es_day in SWAT
    float* m_soilET;
};
#endif /* SEIMS_MODULE_AET_PTH_H */
