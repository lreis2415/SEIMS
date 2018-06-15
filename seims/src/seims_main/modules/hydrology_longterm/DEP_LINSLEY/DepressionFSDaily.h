/*----------------------------------------------------------------------
*	Purpose: 	A simple fill and spill method method to calculate depression storage
*
*	Created:	Junzhi Liu
*	Date:		14-Febrary-2011
*
*	Revision:	Zhiqiang Yu
*   Date:		2011-2-15
*	Description:
*	1.	Modify the name of some parameters, input and output variables.
*		Please see the metadata rules for the names.
*	2.	Depre_in would be DT_Single. Add function SetSingleData() to
*		set its value.
*	3.	This module will be called by infiltration module to get the
*		depression storage. And this module will also use the outputs
*		of infiltration module. The sequence of this two modules is
*		infiltration->depression. When infiltration first calls the
*		depression module, the execute function of depression module
*		is not executed before getting the outputs. So, the output
*		variables should be initial in the Get1DData function. This
*		initialization is realized by function initalOutputs.
*	4.	Delete input D_INFIL and add input D_EXCP.
*
*	Revision:    LiangJunZhu
*	Date    :    2016-7-14
*	Description:
*---------------------------------------------------------------------*/
#ifndef SEIMS_MODULE_DEP_LINSLEY_H
#define SEIMS_MODULE_DEP_LINSLEY_H

#include "SimulationModule.h"

/** \defgroup DEP_LINSLEY
 * \ingroup Hydrology_longterm
 * \brief A simple fill and spill method method to calculate depression storage
 *
 */
/*!
 * \class DepressionFSDaily
 * \ingroup DEP_LINSLEY
 * \brief A simple fill and spill method method to calculate depression storage
 *
 */
class DepressionFSDaily: public SimulationModule {
public:
    DepressionFSDaily();

    ~DepressionFSDaily();

    int Execute() OVERRIDE;

    void SetValue(const char* key, float data) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    bool CheckInputSize(const char* key, int n);

    bool CheckInputData();

private:
    /*!
     * \brief Initialize output variables
     * This module will be called by infiltration module to get the
     *		depression storage. And this module will also use the outputs
     *		of infiltration module. The sequence of this two modules is
     *		infiltration->depression. When infiltration first calls the
     *		depression module, the execute function of depression module
     *		is not executed before getting the outputs. So, the output
     *		variables should be initial in the Get1DData function. This
     *		initialization is realized by function initalOutputs.
     */
    void InitialOutputs();
private:
    /// valid cells number
    int m_nCells;
    /// impound/release
    float* m_impoundTriger;
    /// pothole volume, mm
    float* m_potVol;
    /// initial depression storage coefficient
    float m_depCo;
    /// depression storage capacity (mm)
    float* m_depCap;
    // The embankment area ratio of paddy rice cells
    float m_embnkFr;
    // The fraction of precipitation fall on the embankment that drain into ditches or canals directly
    float m_pcp2CanalFr;
    // landuse
    float* m_landUse;
    /*! Precipitation
    * For STROM_MODE model, the unit is rainfall intensity mm/h
    * For LONGTERM_MODE model, the unit is mm
    */
    float* m_pcp;

    /// pet
    float* m_pet;
    /// evaporation from the interception storage
    float* m_ei;

    /// excess precipitation calculated in the infiltration module
    float* m_pe;

    // state variables (output)

    /// depression storage
    float* m_sd;
    /// evaporation from depression storage
    float* m_ed;
    /// surface runoff
    float* m_sr;
};
#endif /* SEIMS_MODULE_DEP_LINSLEY_H */
