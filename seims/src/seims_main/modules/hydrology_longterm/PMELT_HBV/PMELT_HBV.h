#ifndef SEIMS_MODULE_PMELT_HBV_H
#define SEIMS_MODULE_PMELT_HBV_H

#include "SimulationModule.h"

class PMELT_HBV : public SimulationModule {
public:
    PMELT_HBV(); //! Constructor
    ~PMELT_HBV(); //! Destructor

    ///////////// SetData series functions /////////////

    void SetValue(const char* key, FLTPT value) OVERRIDE;
    void Set1DData(const char* key, int n, FLTPT* data) OVERRIDE;

    ///////////// GetData series functions /////////////
    void Get1DData(const char* key, int* n, FLTPT** data) OVERRIDE;

    ///////////// CheckInputData and InitialOutputs /////////////
    bool CheckInputData() OVERRIDE;
    void InitialOutputs() OVERRIDE;
    bool CheckInputSize(const char* key, int n);

    ///////////// Main control structure of execution code /////////////
    int Execute() OVERRIDE;

private:
    int m_nCells; ///< valid cells number

    //input variables
    FLTPT* m_temp_daily_ave; ///< daily average temperature

    //parameters
    FLTPT m_Ma; ///< Melt factor
    FLTPT m_Ma_min; ///< Minimum melt factor
    FLTPT m_AM; ///< HBV melt aspect correction
    FLTPT m_MRF; ///< HBV melt forest correction
    FLTPT m_Fc; ///< Forest coverage
    FLTPT m_t_melt; ///< Melt temperature
    FLTPT m_slope_corr; ///< Slope correction
    FLTPT m_rain_energy; ///< Rain energy

    //output variables
    FLTPT* m_potentialMelt; ///< Potential Melt
};

#endif /* SEIMS_MODULE_PMELT_HBV_H */