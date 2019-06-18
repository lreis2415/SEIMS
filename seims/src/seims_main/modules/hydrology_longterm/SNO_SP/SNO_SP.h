/*!
 * \file SNO_SP.h
 * \brief Snow melt by snowpack daily method from SWAT.
 *
 * Changelog:
 *   - 1. 2011-05-30 - zq - Initial implementation.
 *   - 2. 2016-05-29 - lj -
 *        -# Remove m_isInitial and add initialOutputs(void)
 *        -# Add m_snowCoverMax and m_snowCover50 to adjust for areal extent of snow cover
 *        -# ReWrite the execute code according to snom.f of SWAT
 *        -# In this version, snow melt is added to net precipitation.
 *
 * \author Zhiqiang Yu, Liangjun Zhu
 */
#ifndef SEIMS_MODULE_SNO_SP_H
#define SEIMS_MODULE_SNO_SP_H

#include "SimulationModule.h"

/*!
 * \defgroup SNO_SP
 * \ingroup Hydrology_longterm
 * \brief Calculate snow melt by snowpack daily method from SWAT
 *
 */

/*!
 * \class SNO_SP
 * \ingroup SNO_SP
 * \brief Calculate snow melt by snowpack daily method from SWAT
 *
 */
class SNO_SP: public SimulationModule {
public:
    //! Constructor
    SNO_SP();

    //! Destructor
    ~SNO_SP();

    void SetValue(const char* key, float data) OVERRIDE;

    void Set1DData(const char* key, int n, float* data) OVERRIDE;

    bool CheckInputData() OVERRIDE;

    void InitialOutputs() OVERRIDE;

    int Execute() OVERRIDE;

    void Get1DData(const char* key, int* n, float** data) OVERRIDE;

private:
    //! Valid cells number
    int m_nCells;
    //! Mean air temperature at which snow melt will occur, sub_smtmp
    float m_t0;
    //! fraction coefficient of precipitation as snow
    float m_kblow;
    //! Snowfall temperature, i.e., precipitation as snow
    float m_snowTemp;
    //! Initial snow water equivalent
    //float m_swe0;

    /*Snow pack temperature lag factor (0-1), sub_timp in SWAT
    * 1 = no lag (snow pack temp=current day air temp)
    * as the lag factor goes to zero, the snow pack's
    * temperature will be less influenced by the current day's
    * air temperature
    */
    float m_lagSnow;
    //! Maximum melt rate for snow during year, sub_smfmx
    float m_csnow6;
    //! Minimum melt rate for snow during year, sub_smfmn
    float m_csnow12;
    //! Minimum snow water content that corresponds to 100% snow cover, mm H2O, SNOCOVMX
    float m_snowCoverMax;
    //! Fraction of SNOCOVMX that corresponds to 50% snow cover, SNO50COV
    float m_snowCover50;
    //! 1st shape parameter for snow cover equation
    float m_snowCoverCoef1;
    //! 2nd shape parameter for snow cover equation
    float m_snowCoverCoef2;

    //! average snow accumulation of the watershed
    //float m_swe;
    //! snow water equivalent of last time step
    //float m_lastSWE;

    //! Mean temperature
    float* m_meanTemp;
    //! Max temperature
    float* m_maxTemp;
    //! Net precipitation
    float* m_netPcp;

    //! snow redistribution
    float* m_snowAccum;
    //! snow sublimation, snoev in SWAT in etact.f
    float* m_SE;

    //! temperature of snow pack, snotmp in SWAT
    float* m_packT;

    /// outputs

    //! amount of water in snow melt, snomlt in SWAT
    float* m_snowMelt;
    //! snow accumulation, sno_hru in SWAT
    float* m_SA;
};
#endif /* SEIMS_MODULE_SNO_SP_H */
