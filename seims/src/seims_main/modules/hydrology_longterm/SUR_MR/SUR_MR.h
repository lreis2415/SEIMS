/*!
 * \file SUR_MR.h
 * \brief Modified Rational Method to calculate infiltration and excess precipitation.
 *
 * Changelog:
 *   - 1. 2011-01-19 - jz - Initial implementation.
 *   - 2. 2011-02-15 - zq -
 *        -# Parameter S_M_frozen would be s_frozen and DT_Single.
 *        -# Parameter sfrozen would be t_soil and in WaterBalance table.
 *        -# Delete parameter Moist_in.
 *        -# Rename the input and output variables. See metadata rules for names.
 *        -# In function execute, do not change m_pNet[i] directly. This will have influence
 *             on another modules who will use net precipitation. Use local variable to replace it.
 *        -# Add API function GetValue.
 *   - 3. 2011-02-19 - jz - Take snowmelt into consideration when calculating PE, PE=P_NET+Snowmelt-F.
 *   - 4. 2013-10-28 - jz - Add multi-layers support for soil parameters.
 *   - 5. 2016-05-27 - lj - Update the support for multi-layers soil parameters.
 *   - 6. 2016-07-14 - lj -
 *        -# Remove snowmelt as AddInput, because snowmelt is considered into net precipitation in SnowMelt moudule,
 *             by the meantime, this can avoid runtime error when SnowMelt module is not configured.
 *        -# Change the unit of soil moisture from mm H2O/mm Soil to mm H2O, which is more rational.
 *        -# Change soil moisture to soil storage which is coincident with SWAT, and do not include wilting point.
 *
 * \author Junzhi Liu, Zhiqiang Yu, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_SUR_MR_H
#define SEIMS_MODULE_SUR_MR_H

#include "SimulationModule.h"

/** \defgroup SUR_MR
 * \ingroup Hydrology_longterm
 * \brief Modified Rational Method to calculate infiltration and excess precipitation
 *
 */

/*!
 * \class SUR_MR
 * \ingroup SUR_MR
 * \brief Modified Rational Method to calculate infiltration and excess precipitation
 *
 */
class SUR_MR: public SimulationModule {
public:
    SUR_MR();

    ~SUR_MR();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    void InitializeIntermediateVariables() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

private:
    /// Hillslope time step (second)
    float m_dt;
    /// count of valid cells
    int m_nCells;
    /// net precipitation of each cell (mm)
    float* m_netPcp;
    /// potential runoff coefficient
    float* m_potRfCoef;

    /// number of soil layers, i.e., the maximum soil layers of all soil types
    int m_maxSoilLyrs;
    /// soil layers number of each cell
    float* m_nSoilLyrs;

    /// mm H2O: (sol_fc) amount of water available to plants in soil layer at field capacity (fc - wp)
    float** m_soilFC;
    /// mm H2O: (sol_ul) amount of water held in the soil layer at saturation (sat - wp water)
    float** m_soilSat;
    /// amount of water held in the soil layer at saturation (sat - wp water), mm H2O, sol_sumul of SWAT
    float* m_soilSumSat;
    /// initial soil water storage fraction related to field capacity (FC-WP)
    float* m_initSoilWtrStoRatio;

    /// Runoff exponent for a near zero rainfall intensity
    float m_rfExp;
    /// Rainfall intensity corresponding to a surface runoff exponent (m_rfExp) of 1
    float m_maxPcpRf;
    /// depression storage (mm)
    float* m_deprSto; // SD(t-1) from the depression storage module

    /// mean air temperature (deg C)
    float* m_meanTemp;

    /// threshold soil freezing temperature (deg C)
    float m_soilFrozenTemp;
    /// frozen soil moisture relative to saturation above which no infiltration occur
    /// (m3/m3 or mm H2O/ mm Soil)
    float m_soilFrozenWtrRatio;
    /// soil temperature obtained from the soil temperature module (deg C)
    float* m_soilTemp;

    /// pothole volume, mm
    float* m_potVol;
    /// impound trigger
    float* m_impndTrig;
    // output
    /// the excess precipitation (mm) of the total nCells, which could be depressed or generated surface runoff
    float* m_exsPcp;
    /// infiltration map of watershed (mm) of the total nCells
    float* m_infil;
    /// soil water storage (mm)
    float** m_soilWtrSto;
    /// soil water storage in soil profile (mm)
    float* m_soilWtrStoPrfl;
};
#endif /* SEIMS_MODULE_SUR_MR_H */
