/*
 * \brief Non point source management
 * \author Liang-Jun Zhu
 * \date Jul. 2016
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

    int Execute() OVERRIDE;

    void SetValue(const char* key, float value) OVERRIDE;

    void Set2DData(const char* key, int n, int col, float** data) OVERRIDE;

    void SetScenario(Scenario* sce) OVERRIDE;

private:
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
