/*!
 * \file SET_LM.h
 * \brief The method of soil actual ET linearly with actual soil moisture developed by
 *          Thornthwaite and Mather (1955), which was also adapted by WetSpa Extension.
 *
 * Changelog:
 *   - 1. 2018-06-26 - lj - Remove Wilting point since SOL_AWC is preprocessed by FC-WP.
 *
 * \author Chunping Ou, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_SET_LM_H
#define SEIMS_MODULE_SET_LM_H

#include "SimulationModule.h"

/*!
 * \defgroup SET_LM
 * \ingroup Hydrology_longterm
 * \brief Calculate soil Temperature according to the linearly relationship with actual soil moisture
 */

/*!
 * \class SET_LM
 * \ingroup SET_LM
 * \brief Calculate soil Temperature according to the linearly relationship with actual soil moisture
 */
class SET_LM: public SimulationModule {
public:
    SET_LM();

    ~SET_LM();

    void SetValue(const char* key, float value) OVERRIDE;

    void Set1DData(const char* key, int nRows, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* nrows, float** data) OVERRIDE;

private:
    int m_nCells; ///< valid cells number
    int m_maxSoilLyrs; ///< maximum number of soil layers
    float* m_nSoilLyrs; ///< Soil layers number
    float** m_soilThk;  ///< Soil thickness of each layer, mm

    float** m_soilWtrSto; ///< soil moisture, mm
    float** m_soilFC;     ///< field capacity (FC-WP, same as SWAT), mm
    float* m_pet;         ///< Potential evapotranspiration
    float* m_IntcpET;     ///< Evaporation from interception
    float* m_deprStoET;   ///< Evaporation from depression storage
    float* m_maxPltET;    ///< Evaporation from plant

    float* m_soilTemp;      ///< Soil temperature
    float m_soilFrozenTemp; ///< Freezing temperature

    float* m_soilET; ///< Output, actual soil evaporation
};
#endif /* SEIMS_MODULE_SET_LM_H */
