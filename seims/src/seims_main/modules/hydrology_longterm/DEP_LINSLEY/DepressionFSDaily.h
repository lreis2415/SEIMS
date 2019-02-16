/*!
 * \file DepressionFSDaily.h
 * \brief A simple fill and spill method method to calculate depression storage.
 *
 * Changelog:
 *   - 1. 2011-02-14 - jz - Initial implementation.
 *   - 2. 2011-02-15 - zq -
 *        -# Modify the name of some parameters, input and output variables.
 *		       Please see the metadata rules for the names.
 *        -# Depre_in would be DT_Single. Add function SetSingleData() to set its value.
 *        -# This module will be called by infiltration module to get the
 *		       depression storage. And this module will also use the outputs
 *		       of infiltration module. The sequence of this two modules is
 *		       infiltration->depression. When infiltration first calls the
 *		       depression module, the execute function of depression module
 *		       is not executed before getting the outputs. So, the output
 *		       variables should be initial in the Get1DData function. This
 *		       initialization is realized by function initalOutputs.
 *        -# Delete input D_INFIL and add input D_EXCP.
 *   - 3. 2016-07-14 - lj - Code review and reformat.
 *
 * \author Junzhi Liu, Zhiqiang Yu, Liangjun Zhu
 */
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

    void SetValue(const char* key, float value) OVERRIDE;

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
    
    /// 
    float* m_netPcp;
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
