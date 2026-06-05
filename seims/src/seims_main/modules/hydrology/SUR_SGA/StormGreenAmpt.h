/*!
 * \brief Green-Ampt Method to calculate infiltration and excess precipitation
 * \author Junzhi Liu, Liang-Jun Zhu
 *
 * Changelog:
 *   - 1. 2011-10-30 - jz - Original implementation.
 *   - 2. 2021-10-29 - lj - Update to new SEIMS designs and code styles.
 *
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

    void SetValue(const char* key, int value) OVERRIDE;

    void SetValue(const char* key, FLTPT value) OVERRIDE;

    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    void Set1DData(const char* key, int n, int* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

    ///**
    //*	@brief check the output data. Make sure all the output data is available.
    //*
    //*	@param output1: the output variable PE
    //*	@param output2: the next output variable infiltration
    //*	@return bool The validity of the output data.
    //*/
    //bool CheckOutputData(float* output1, float* output2);

    void clearInputs(void);

private:
    int output_icell_max;
    int output_icell_min;
    int printInfilMinT;
    int printInfilMaxT;
    int counter;
    std::ofstream  infiltFileFptr;

    /// this function calculated the wetting front matric potential
    float CalculateCapillarySuction(float por, float clay, float sand);

    /// Calculate the active storm infiltration depth (mm)
    float CalculateWettingFrontDepth(int cell);

    /// Calculate remaining event-scale pore deficit within the active depth (mm)
    float CalculateActiveInfilCap(int cell, float activeDepth);

    /// Calculate current and event-reference storage deficits inside the active depth (mm)
    void CalculateActiveStorage(int cell, float activeDepth, float& dynamicStorage, float& eventStorage);

    /// Redistribute Green-Ampt event memory during rainfall breaks (mm)
    float RedistributeAccumulatedInfiltration(int cell, float activeDepth, float dt, bool canRedistribute);

    /// Calculate initial volumetric soil water content for a layer
    float CalculateInitialSoilWater(int cell, int layer);

    /// Add infiltration water to soil layers within the active wetting front depth
    void AddInfiltrationToSoil(int cell, float infiltration, float activeDepth);

    // Parameters from database
    float m_dt;             ///< time step (seconds)
    int m_nCells;           ///< valid cells number
    float m_tSnow;          ///< snow fall temperature
    float m_t0;             ///< snow melt threshold temperature
    float m_infilFactor;    ///< infiltration reduction factor, default 1.0
    float m_activeDepthMax; ///< maximum active wetting front depth (mm)
    float m_moistureReference; ///< 0: MOIST_IN relative to FC; 1: relative to porosity
    float m_accumuRecoveryRate; ///< dry-period recovery rate of Green-Ampt cumulative infiltration (mm/h)
    float m_accumuRecoveryDelay; ///< dry duration before recovery starts (h)
    float m_stateRecoveryFactor; ///< fraction of dry-period state-based Green-Ampt memory recovery (0-1)
    int m_maxSoilLyrs;      ///< maximum soil layers, mlyr in SWAT
    int* m_nSoilLyrs;     ///< soil layers
    float** m_soilDepth;    ///< root depth
    float** m_soilPor;      ///< soil porosity
    float** m_soilClay;     ///< percent of clay content
    float** m_soilSand;     ///< percent of sand content
    float** m_ks;           ///< saturated hydraulic conductivity
    ///< initial soil water storage fraction related to field capacity (FC-WP)
    float* m_initSoilWtrStoRatio;
    //float** m_soilWtrSto;   ///< initial soil moisture
    float** m_soilFC;     ///< field capacity

    // Inputs from other modules
    float* m_meanTmp;  ///< mean temperature
    float* m_netPcp;   ///< net precipitation
    float* m_sd;  ///< depression storage
    float* m_snowMelt; ///< snow melt (mm)
    float* m_snowAccu; ///< snow accumulation (mm)
    float* m_surfRf;   ///< surface water depth

    // intermediate variables
    float* m_capillarySuction; ///< Soil Capillary Suction Head (m)
    float* m_accumuDepth;      ///< cumulative infiltration depth (mm)
    float* m_dryDuration;      ///< continuous dry duration (s)

    // Outputs
    float** m_soilWtrSto; ///< soil moisture
    float* m_infil; ///< infiltration
    float* m_infilCapacitySurplus; ///< surplus of infiltration capacity
    /// the excess precipitation (mm) of the total nCells, which could be depressed or generated surface runoff
    FLTPT* m_exsPcp;
};
#endif /* SEIMS_SUR_SGA_H */
