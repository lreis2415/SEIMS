/*!
 * \file SOL_WB.h
 * \brief Check soil water balance.
 *
 * Changelog:
 *   - 1. 2016-07-28 - lj - Move subbasin class to base/data/clsSubbasin, to keep consistent with other modules.
 *
 * \author Chunping Ou, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_SOL_WB_H
#define SEIMS_MODULE_SOL_WB_H

#include "SimulationModule.h"
#include "clsSubbasin.h"

/*!
 * \defgroup SOL_WB
 * \ingroup Hydrology_longterm
 * \brief Soil water balance calculation
 *
 */

/*!
 * \class SOL_WB
 * \ingroup SOL_WB
 * \brief Soil water balance calculation
 *
 */

class SOL_WB: public SimulationModule {
public:
    SOL_WB();

    ~SOL_WB();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int nrows, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    void SetSubbasins(clsSubbasins* subbasins) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

private:
    /*!
     * \brief Set parameter values to subbasins
     */
    void SetValueToSubbasins();
private:
    //! valid cells number
    int m_nCells;
    //! maximum soil layers number
    int m_maxSoilLyrs;
    //! soil layers number of each cell
    float* m_nSoilLyrs;
    //! soil thickness of each layer
    float** m_soilThk;
    //! the maximum soil depth
    float* m_soilMaxRootD;

    //! Net precipitation (include snow melt if stated) (mm)
    float* m_netPcp;
    //! infiltration water (mm)
    float* m_infil;
    //! evaporation from the soil water storage, es_day in SWAT (mm)
    float* m_soilET;
    //! revaporization from groundwater to the last soil layer (mm)
    float* m_Revap;
    //! subsurface runoff
    float** m_subSurfRf;
    //! percolation (mm)
    float** m_soilPerco;
    //! soil storage (mm)
    float** m_soilWtrSto;
    // Outputs
    // used to output time series result for soil water balance

    //! precipitation on the current day (mm)
    float* m_PCP;
    //! interception loss (mm)
    float* m_intcpLoss;
    //! evaporation from the interception storage (mm)
    float* m_IntcpET;
    //! depression (mm)
    float* m_deprSto;
    //! evaporation from depression storage (mm)
    float* m_deprStoET;
    //! surface runoff generated (mm)
    float* m_surfRf;
    //! groundwater runoff
    float* m_RG;
    //! snow sublimation
    float* m_snowSublim;
    //! mean temperature
    float* m_meanTemp;
    //! soil temperature
    float* m_soilTemp;
    //! subbasins number
    int m_nSubbsns;
    //! subbasin IDs
    vector<int> m_subbasinIDs;
    //! All subbasins information
    clsSubbasins* m_subbasinsInfo;
    /* soil water balance, time series result
    * the row index is subbasinID
    */
    float** m_soilWtrBal;
};
#endif /* SEIMS_MODULE_SOL_WB_H */
