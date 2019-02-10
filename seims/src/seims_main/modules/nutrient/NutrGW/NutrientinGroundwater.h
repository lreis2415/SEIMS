/*!
 * \file NutrientinGroundwater.h
 * \brief Calculates the nitrate and soluble phosphorus loading contributed by groundwater flow.
 *
 * Changelog:
 *   - 1. 2016-06-30 - hr - Initial implementation.
 *   - 2. 2018-03-23 - lj - Debug for mpi version.
 *   - 3. 2018-05-15 - lj - Code review and reformat.
 *
 * \author Huiran Gao, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_NUTRGW_H
#define SEIMS_MODULE_NUTRGW_H

#include "SimulationModule.h"

/** \defgroup NutrGW
 * \ingroup Nutrient
 * \brief Calculates the nitrate and soluble phosphorus loading contributed by groundwater flow.
 */

/*!
 * \class NutrientinGroundwater
 * \ingroup NutrGW
 *
 * \brief Calculates the nitrate and soluble phosphorus loading contributed by groundwater flow.
 *
 */

class NutrientinGroundwater: public SimulationModule {
public:
    NutrientinGroundwater();

    ~NutrientinGroundwater();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nRows, int nCols, float** data) OVERRIDE;

    void SetReaches(clsReaches* reaches) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void SetSubbasins(clsSubbasins* subbasins) OVERRIDE;

    TimeStepType GetTimeStepType() OVERRIDE{ return TIMESTEP_CHANNEL; }

private:
    /// current subbasin ID, 0 for the entire watershed
    int m_inputSubbsnID;
    /// cell width of grid map (m)
    float m_cellWth;
    /// number of cells
    int m_nCells;
    /// time step (s)
    int m_TimeStep;

    /// input data
    /// gw0
    float m_gw0;
    /// nitrate N concentration in groundwater loading to reach (mg/L, i.e. g/m3)
    float* m_gwNO3Conc;
    /// kg
    float* m_gwNO3;
    /// soluble P concentration in groundwater loading to reach (mg/L, i.e. g/m3)
    float* m_gwSolPConc;
    /// kg
    float* m_gwSolP;
    /// groundwater contribution to stream flow (m3/s)
    float* m_gw_q;
    /// groundwater storage
    float* m_gwStor;
    /// amount of nitrate percolating past bottom of soil profile, kg
    float* m_perco_no3_gw;
    /// amount of solute P percolating past bottom of soil profile, kg
    float* m_perco_solp_gw;

    // soil related
    /// amount of nitrogen stored in the nitrate pool in soil layer
    float** m_soilNO3;
    /// amount of soluble phosphorus stored in the soil layer
    float** m_soilSolP;
    /// max number of soil layers
    int m_maxSoilLyrs;
    /// number of soil layers of each cell
    float* m_nSoilLyrs;


    /// outputs

    /// nitrate loading to reach in groundwater to channel
    float* m_gwNO3ToCh;
    /// soluble P loading to reach in groundwater to channel
    float* m_gwSolPToCh;

    /// subbasin related
    /// the total number of subbasins
    int m_nSubbsns;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    /// subbasin grid (subbasins ID)
    float* m_subbsnID;
    /// subbasins information
    clsSubbasins* m_subbasinsInfo;
};
#endif /* SEIMS_MODULE_NUTRGW_H */
