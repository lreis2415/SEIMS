/**
*	@author    Junzhi Liu
*	@date	19-January-2011
*
*	@brief	Modified Rational Method to calculate infiltration and excess precipitation
*
*	Revision:	Zhiqiang Yu
*   Date:		2011-2-15
*	Description:
*	1.	Parameter S_M_frozen would be s_frozen and DT_Single.
*	2.	Parameter sfrozen would be t_soil and in WaterBalance table.
*	3.  Delete parameter Moist_in.
*	4.  Rename the input and output variables. See metadata rules for names.
*	5.  In function execute, do not change m_pNet[i] directly. This will have influence
*		on another modules who will use net precipitation. Use local variable to replace it.
*	6.  Add API function GetValue.
*
*	Revision:	Junzhi Liu
*   Date:		2011-2-19
*	1.	Rename m_excess to m_pe
*	2.	Take snowmelt into consideration when calculating PE, PE=P_NET+Snowmelt-F
*
*	Revision:	Junzhi Liu
*   Date:		2013-10-28
*	1.	Add multi-layers support for soil parameters
*
*   Revision: LiangJun Zhu
*   Date:        2016-5-27
*   1. Update the support for multi-layers soil parameters
*
*   Revision: LiangJun Zhu
*   Date:       2016-7-14
*   1. Remove snowmelt as AddInput, because snowmelt is considered into net precipitation in SnowMelt moudule,
*      by the meantime, this can avoid runtime error when SnowMelt module is not configured.
*   2. Change the unit of soil moisture from mm H2O/mm Soil to mm H2O, which is more rational.
*   3. Change soil moisture to soil storage which is coincident with SWAT, and do not include wilting point.
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

    int Execute() OVERRIDE;

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nRows, int* nCols, float*** data) OVERRIDE;

    bool CheckInputSize(const char* key, int n);

    void CheckInputData();

    /// initial output for the first run
    void InitialOutputs();
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

    /// runoff exponent
    float m_rfExp;
    /// maximum precipitation corresponding to runoffCo
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
