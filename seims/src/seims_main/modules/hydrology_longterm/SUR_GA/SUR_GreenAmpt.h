/**
*	@file
*	@version	1.0
*	@author		Wu Hui
*	@date	30-December-2010
*
*	@brief	Green-Ampt Method to calculate infiltration and excess precipitation
*
*	Revision: Wu Hui
*   Date:     2011-2-16
*   1. Because w1 and w2 in equation 2:1.1.6 is fixed, add two variable to store these
*      two coeffiences to avoid repeating calculation.
*   2. When w1 and w2 is calculated using equation 2:1.1.7 and 2:1.1.8, FC and SAT should
*	   be FC*rootdepth and SAT*rootdepth rather than (FC-WP)*rootdepth and (SAT-WP)*rootdepth.
*	   And the unit conversion of rootdepth would be from m to mm.
*   3. Overload the funtion Calculate_CN to calculate the CN of one cell.
*   4. Delete some parameters and input variables. They are Depre_in,Moist_in and Depression.
*      D_SOMO from soil water balance module and D_DPST from depression module is enough. The
*      initalization of soil moisture and depression storage is the task of soil water balance
*	   module and depression module.
*	5. Modify the name of input and output variables to make it consistent with other modules.
*
*	Revision:	Zhiqiang Yu
*	Date:		2011-2-16
*	Description
*	1.	The wetting front matric potential is a constant for a specific cell. It just needs to
*		be calculated once at the beginning. So, Add a local variable m_wfmp to store its value
*		and a function initialWFMP to calculate its value. Wu Hui, please note that when calculating
*		WFMP using equation 2:1.2.5 in SWAT, the unit of clay and sand is percent. You do not need
*		to divided it by 100. See soil_phys.f in SWAT source code for detail reference.
*	2.	The unit of time step passed from main program is hour rather than minute. So, it need to
*		be converted to minutes in function SetValue.
*	3.	Rename some names of keys in function Set1DData.
*	4.	Delete parameter INFRate_In and set the initial value of infiltration rate to 0.
*	5.  Add API function GetValue.
*	6.	Delete three local variables: m_julianDay, m_Depre_in, m_Depression.
*	7.	Correct the logic of function Get1DData.
*/
#ifndef SEIMS_SUR_GA_H
#define SEIMS_SUR_GA_H

#include "SimulationModule.h"

// using namespace std;  // Avoid this statement! by lj.

/*!
 * \defgroup SUR_GA
 * \ingroup Hydrology_longterm
 * \brief Green-Ampt Method to calculate infiltration and excess precipitation
 *
 */

/*!
 * \class SUR_GreenAmpt
 * \ingroup SUR_GA
 * \brief Green-Ampt Method to calculate infiltration and excess precipitation
 *
 */
class SUR_GreenAmpt : public SimulationModule {
public:
    SUR_GreenAmpt(void);

    ~SUR_GreenAmpt(void);

    virtual int Execute(void);

    virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    bool CheckInputSize(const char *key, int n);

    bool CheckInputData(void);

private:

    /// count of valid cells
    int m_cellSize;
    /// length of time step used to report precipitation data for sub-daily modeling (minutes)
    float m_TimeStep;
    /// saturated hydraulic conductivity from parameter database (mm/h)
    float *m_Conductivity;
    /// soil porosity
    float *m_porosity;
    /// percent of clay content from parameter database
    float *m_clay;
    /// percent of sand content from parameter database
    float *m_sand;
    /// root depth of plants (m)
    float *m_rootDepth;
    /// CN under moisture condition II
    float *m_cn2;
    /// Net precipitation calculated in the interception module (mm)
    float *m_P_NET;
    /// water content of soil at field capacity
    float *m_fieldCap;
    /// plant wilting point moisture
    float *m_wiltingPoint;
    /*/// initial soil moisture
    float* m_Moist_in;*/
    /// soil moisture of each time step
    float *m_soilMoisture;
    /*/// initial infiltration rate
    float* m_INFRate_in;*/
    /// initial infiltration rate or infiltration rate of watershed (mm/hr) at previous time step (t-1)
    float *m_INFRate;
    /// Initial depression storage coefficient
    ///float m_Depre_in;
    /// Depression storage capacity
    ///float* m_Depression;
    /// depression storage
    float *m_SD;    // SD(t-1) from the depression storage module
    /// from interpolation module
    /// air temperature of the current day
    float *m_tMin, *m_tMax;
    /// snowfall temperature from the parameter database (��)
    float m_Tsnow;
    /// threshold soil freezing temperature (��)
    float m_Tsoil;
    /// frozen soil moisture relative to saturation above which no infiltration occur (m3/m3)
    float m_Sfrozen;
    /// snowmelt threshold temperature from the parameter database (��)
    float m_T0;
    /// snowmelt from the snowmelt module  (mm)
    float *m_SM;
    /// snow accumulation from the snow balance module (mm) at t+1 timestep
    float *m_SA;
    /// soil temperature obtained from the soil temperature module (��)
    float *m_TS;
    /// array containing the row and column numbers for valid cells
    float **m_mask;
    /// from GIS interface Project/model subdirectory
    /// Julian day
    ///int m_julianDay;

    ///output values
    /// the excess precipitation (mm) of the total nCells
    float *m_PE;
    /// infiltration map of watershed (mm) of the total nCells
    float *m_INFIL;

    time_t m_date;

    //add by Wu Hui  2011-02-16
    float *m_w1;
    float *m_w2;
    float *m_sMax;

    void initalW1W2(void);

    float Calculate_CN(int cell);

    /**
    *	@calculates wetting front matric potential.
    *
    *	@param float sol_pol: soil porosity
    *	@param float sol_clay: the percent clay content
    *	@param float sand: the percent sand content
    *	@return wetting front matric potential
    */
    float Calculate_WFMP(float sol_pol, float sol_clay, float sand);

    //add by Zhiqiang 2011-02-16
    float *m_wfmp;

    void initialWFMP(void);

    void InitalOutputs(void);
};
#endif /* SEIMS_SUR_GA_H */
