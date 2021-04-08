/*!
 * \file NonPointSource_Management.h
 * \brief Non point source management.
 *
 * Changelog:
 *   - 1. 2016-07-30 - lj - Initial implementation.
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_MODULE_NPSMGT_H
#define SEIMS_MODULE_NPSMGT_H

#include "SimulationModule.h"

/** \defgroup NPSMGT
 * \ingroup Management
 * \brief Non point source management
 */
/*!
 * \class NPS_Management
 * \ingroup NPSMGT
 * \brief All management operation in SWAT, e.g., plantop, killop, harvestop, etc.
 */
class NPS_Management: public SimulationModule {
public:
    NPS_Management();

    ~NPS_Management();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    int Execute() OVERRIDE;

private:
    /// valid cells number
    int m_nCells;
    /// cell width (m)
    float m_cellWth;
    /// area of cell (m^2)
    float m_cellArea;
    /// time step (second)
    float m_timestep;
    /// management fields raster
    float* m_mgtFields;
    /*!
     * areal source operations
     * key: unique index, BMPID * 100000 + subScenarioID
     * value: areal source management factory instance
     */
    map<int, BMPArealSrcFactory *> m_arealSrcFactory;

    /// variables to be updated (optionals)

    /// water storage of soil layers
    float** m_soilWtrSto;
    /// nitrate kg/ha
    float** m_soilNO3;
    /// ammonium kg/ha
    float** m_soilNH4;
    /// soluble phosphorus kg/ha
    float** m_soilSolP;
    float** m_soilStabOrgN;
    float** m_soilHumOrgP;
};
#endif /* SEIMS_MODULE_NPSMGT_H */
