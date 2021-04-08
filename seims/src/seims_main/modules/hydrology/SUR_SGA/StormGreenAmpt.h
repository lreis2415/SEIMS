/*!
 * \brief Green Ampt Method to calculate infiltration and excess precipitation
 * \author Junzhi Liu, Liang-Jun Zhu
 * 
 * Changelog:
 *   - 1. 2021-04-08 - lj - Update to new SEIMS designs and code styles.
 *   
 * \date Oct. 2011
 */
#ifndef SEIMS_SUR_SGA_H
#define SEIMS_SUR_SGA_H

#include "SimulationModule.h"

/** \defgroup SUR_SGA
 * \ingroup Hydrology
 * \brief  Green-Ampt Method to calculate infiltration and excess precipitation
 */

/*!
 * \class StormGreenAmpt
 * \ingroup SUR_SGA
 *
 * \brief Green-Ampt Method to calculate infiltration and excess precipitation
 *
 */
class StormGreenAmpt: public SimulationModule {
public:
    StormGreenAmpt();

    ~StormGreenAmpt();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nRows, int* nCols, float*** data) OVERRIDE;

private:

    /// Calculated the wetting front matric potential
    float CalculateCapillarySuction(float por, float clay, float sand);

    /// Parameters from database
    float m_dt;             ///< time step (seconds)
    int m_nCells;           ///< valid cells number
    int m_maxSoilLyrs;      ///< maximum soil layers, mlyr in SWAT
    float* m_nSoilLyrs;     ///< soil layers
    float** m_rootDepth;    ///< root depth
    float** m_porosity;     ///< soil porosity
    float** m_soilMoisture; ///< soil moisture
    float** m_clay;         ///< percent of clay content
    float** m_sand;         ///< percent of sand content
    float** m_ks;           ///< saturated hydraulic conductivity 
    float** m_initSoilMoisture; ///< initial soil moisture
    float** m_fieldCap;     ///< field capacity
    float m_tSnow;          ///< snow fall temperature
    float m_t0;             ///< snow melt threshold temperature
    float m_tSoilFrozen;    ///< threshold soil freezing temperature
    float m_sFrozen;        ///< frozen soil moisture above which no infiltration occurs (m3/m3)

    /// Inputs from other modules
    float* m_pNet;     ///< net precipitation
    float* m_sd;       ///< depression storage
    float* m_tMax;     ///< maximum temperature
    float* m_tMin;     ///< minimum temperature
    float* m_soilTemp; ///< soil temperature
    float* m_snowMelt; ///< snow melt (mm)
    float* m_snowAccu; ///< snow accumulation (mm)
    float* m_sr;       ///< surface water depth

    /// intermediate variables
   
    float* m_capillarySuction; ///< Soil Capillary Suction Head (m)
    float* m_accumuDepth;      ///< cumulative infiltration depth (m)

    /// Outputs
    float* m_infil; ///< infiltration
    float* m_infilCapacitySurplus; ///< surplus of infiltration capacity
};
#endif /* SEIMS_SUR_SGA_H */
