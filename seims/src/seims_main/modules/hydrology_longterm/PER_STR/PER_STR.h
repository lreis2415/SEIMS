/*!
 * \file PER_STR.h
 * \brief Percolation calculated by storage routing method
 * reference SWAT theory manual, p151-152
 *
 * Changelog:
 *   - 1. 2011-05-30 - jz - Initial implementation.
 *   - 2. 2016-09-08 - lj - ReWrite according to percmain.f and sat_excess.f of SWAT
 *
 * \author Liangjun Zhu, Junzhi Liu
 */
#ifndef SEIMS_MODULE_PER_STR_H
#define SEIMS_MODULE_PER_STR_H

#include "SimulationModule.h"

/*!
 * \defgroup PER_STR
 * \ingroup Hydrology_longterm
 * \brief Percolation calculated by storage routing method.
 */

/*!
 * \class PER_STR
 * \ingroup PER_STR
 * \brief Percolation calculated by storage routing method.
 *
 */
class PER_STR: public SimulationModule {
public:
    PER_STR();

    ~PER_STR();

    void SetValue(const char* key, float data) OVERRIDE;

    void Set1DData(const char* key, int nRows, float* data) OVERRIDE;

    void Set2DData(const char* key, int nrows, int ncols, float** data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get2DData(const char* key, int* nrows, int* ncols, float*** data) OVERRIDE;

private:
    /// maximum number of soil layers
    int m_maxSoilLyrs;
    /// soil layers
    float* m_nSoilLyrs;
    /// soil thickness
    float** m_soilThk;
    /// time step
    int m_dt;
    /// valid cells number
    int m_nCells;
    /// threshold soil freezing temperature
    float m_soilFrozenTemp;
    /// saturated conductivity
    float** m_ks;
    /// amount of water held in the soil layer at saturation (sat - wp water), mm
    float** m_soilSat;
    /// amount of water held in the soil layer at field capacity (fc - wp water) mm H2O
    float** m_soilFC;
    /// soil moisture, mm H2O
    float** m_soilWtrSto;
    /// amount of water stored in soil profile on current day, sol_sw in SWAT
    float* m_soilWtrStoPrfl;
    /// soil temperature
    float* m_soilTemp;
    /// infiltration, mm
    float* m_infil;
    /// surface runoff, mm
    float* m_surfRf;
    /// pothole volume, mm
    float* m_potVol;
    /// impounding trigger
    float* m_impoundTrig;
    /// Output: percolation
    float** m_soilPerco;
};
#endif /* SEIMS_MODULE_PER_STR_H */
