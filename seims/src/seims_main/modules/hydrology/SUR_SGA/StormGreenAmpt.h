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
#define IS_DEBUG 1
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

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;


	void deleteExistFile(string file);

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

    // Parameters from database
    float m_dt;             ///< time step (seconds)
    int m_nCells;           ///< valid cells number
    float m_tSnow;          ///< snow fall temperature
    float m_t0;             ///< snow melt threshold temperature
    int m_maxSoilLyrs;      ///< maximum soil layers, mlyr in SWAT
    float* m_nSoilLyrs;     ///< soil layers
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
    float* m_deprSto;  ///< depression storage
    float* m_snowMelt; ///< snow melt (mm)
    float* m_snowAccu; ///< snow accumulation (mm)
    float* m_surfRf;   ///< surface water depth

    // intermediate variables
    float* m_capillarySuction; ///< Soil Capillary Suction Head (m)
    float* m_accumuDepth;      ///< cumulative infiltration depth (m)

    // Outputs
    float** m_soilWtrSto; ///< soil moisture
    float* m_infil; ///< infiltration
    float* m_infilCapacitySurplus; ///< surplus of infiltration capacity
};
#endif /* SEIMS_SUR_SGA_H */
