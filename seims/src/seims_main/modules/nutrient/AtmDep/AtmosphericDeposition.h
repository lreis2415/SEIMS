/*!
 * \file AtmosphericDeposition.h
 * \brief Add nitrate from rainfall to the soil profile as in SWAT rev. 637, nrain.f
 *
 * Changelog:
 *   - 1. 2016-05-30 - hr - Initial implementation.
 *   - 2. 2016-07-24 - lj -
 *        -# Delete m_cellWidth, m_nSoilLayers, m_sol_z, which are useless.
 *        -# Change m_wshd_rno3 to store nitrate from rainfall of current day.
 *        -# Remove output of m_sol_no3, which is redundant and unnecessary.
 *   - 3. 2018-05-15 - lj - Code review and reformat.
 *
 * \author Huiran Gao, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_ATMDEP_H
#define SEIMS_MODULE_ATMDEP_H

#include "SimulationModule.h"

/** \defgroup ATMDEP
 * \ingroup Nutrient
 * \brief Calculate the atmospheric deposition of nitrogen, include nitrate and ammonia.
 */
/*!
 * \class AtmosphericDeposition
 * \ingroup ATMDEP
 */
class AtmosphericDeposition: public SimulationModule {
public:
    AtmosphericDeposition();

    ~AtmosphericDeposition();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    int Execute() OVERRIDE;

private:
    /// size of array
    int m_nCells;
    /// maximum soil layers
    int m_maxSoilLyrs;

    /// parameters

    /// concentration of nitrate in the rain (mg N/L)
    float m_rainNO3Conc;
    /// concentration of ammonia in the rain (mg N/L)
    float m_rainNH4Conc;
    ///atmospheric dry deposition of nitrates (kg/ha)
    float m_dryDepNO3;
    ///atmospheric dry deposition of ammonia (kg/ha)
    float m_dryDepNH4;

    /// inputs

    /// precipitation (mm H2O)
    float* m_pcp;
    ///// root depth from the soil surface
    //float **m_sol_z;

    ///amount of ammonium in layer (kg/ha)
    float** m_soilNH4;
    /// amount of nitrate in layer (kg/ha)
    float** m_soilNO3;
};
#endif /* SEIMS_MODULE_ATMDEP_H */
