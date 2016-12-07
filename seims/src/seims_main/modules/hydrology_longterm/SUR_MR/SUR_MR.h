/** 
*	@file
*	@version	1.3
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

#pragma once

#include <string>
#include <ctime>
#include "api.h"

using namespace std;

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
class SUR_MR : public SimulationModule
{
public:
    //! Constructor
    SUR_MR(void);

    //! Destructor
    ~SUR_MR(void);

    virtual int Execute();

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int nrows, int ncols, float **data);

    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data);

    bool CheckInputSize(const char *key, int n);

    void CheckInputData(void);

private:
    /// Hillslope time step (second)
    float m_dt;
    /// count of valid cells
    int m_nCells;
    /// net precipitation of each cell (mm)
    float *m_pNet;
    /// potential runoff coefficient
    float *m_runoffCo;

    /// number of soil layers, i.e., the maximum soil layers of all soil types
    int m_nSoilLayers;
    /// soil layers number of each cell
    float *m_soilLayers;
 //   /// soil depth (mm)
 //   float **m_soilDepth;
	///// soil thickness (mm)
	//float **m_soilThick;

 //   /// soil porosity, mm H2O/mm Soil
 //   float **m_porosity;
 //   /// water content of soil at field capacity, mm H2O/mm Soil
	//float **m_fieldCap;
	///// water content of soil at wilting point, mm H2O/mm Soil
	//float **m_wiltingPoint;
	
	///mm H2O: (sol_fc) amount of water available to plants in soil layer at field capacity (fc - wp)
	float **m_sol_awc;
	/// amount of water held in the soil layer at saturation (sat - wp water), mm H2O, sol_sumul of SWAT
	float *m_sol_sumsat;
    /// initial soil water storage fraction related to field capacity (FC-WP)
    float *m_initSoilStorage;

    /// runoff exponent
    float m_kRunoff;
    /// maximum precipitation corresponding to runoffCo
    float m_pMax;
    /// depression storage (mm)
    float *m_sd;    // SD(t-1) from the depression storage module

    /// mean air temperature (deg C)
    float *m_tMean;
    ///// snow fall temperature (deg C)
    //float m_tSnow;
    ///// snow melt threshold temperature (deg C)
    //float m_t0;
    ///// snow melt from the snow melt module  (mm)
    //float *m_snowMelt;
    ///// snow accumulation from the snow balance module (mm) at t+1 time step
    //float *m_snowAccu;

    /// threshold soil freezing temperature (deg C)
    float m_tFrozen;
    /// frozen soil moisture relative to saturation above which no infiltration occur 
    /// (m3/m3 or mm H2O/ mm Soil)
    float m_sFrozen;
    /// soil temperature obtained from the soil temperature module (deg C)
    float *m_soilTemp;

	/// pothole volume, mm
	float *m_potVol;
	/// impound trigger
	float *m_impoundTrig;
    // output
    /// the excess precipitation (mm) of the total nCells, which could be depressed or generated surface runoff
    float *m_pe;
    /// infiltration map of watershed (mm) of the total nCells
	float *m_infil;
	/// soil water storage (mm)
	float **m_soilStorage;
	/// soil water storage in soil profile (mm)
	float *m_soilStorageProfile;

    /// initial output for the first run
    void initialOutputs();
};


/*/// maximum temperature
float *m_tMax;
/// minimum temperature
float *m_tMin;*/